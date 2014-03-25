#ifndef __WAIT_FOR_MULTIPLE_OBJECTS_H_
#define __WAIT_FOR_MULTIPLE_OBJECTS_H_

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <vector>
#include <map>
#include <stddef.h>

#if !(defined(_WIN32) || defined(_WIN64))
#ifdef FI_LINUX
#include <semaphore.h>
#endif //FI_LINUX

#include <pthread.h>

#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/signal.h>

#include "W2KWrapBase.h"
using namespace std;

#define infinite					 -1
#define wait_success				  0
#define wait_error					 -1
#define invalid_wait_object			 -1
#define wait_timeout				 -1
#define system_v_semaphore_key_begin 0xaaaaaaaa

#define safe_delete_array(delete_array_pointer) { if (delete_array_pointer != NULL) { delete [] delete_array_pointer; delete_array_pointer = NULL; } }
#define safe_delete_pointer(delete_pointer) { if (delete_pointer != NULL) { delete delete_pointer; delete_pointer = NULL; } }
#define check_new_valid(p_parameter) { if (p_parameter == NULL) { return wait_error; } }


class c_lock
{
public:
	c_lock();
	~c_lock();

public:
	void lock();
	void unlock();
    bool islock()
    {
        bool bret= false;

        pthread_mutex_lock(&m_self_mutex);
        bret = m_n_reference_count > 0 ? true : false;
        pthread_mutex_unlock(&m_self_mutex);

        return bret;
    }

public:
	pthread_t get_current_owner();

private:
	pthread_mutex_t m_mutex;
	pthread_mutex_t m_self_mutex;
	pthread_t m_owner_thread_id;
	unsigned int m_n_reference_count;
};

class c_wait_object
{
public:
	virtual ~c_wait_object(){};

public:
	virtual int  set() = 0;
	virtual int  reset() = 0;

public:
	virtual int  check() = 0;
	virtual int  wait(unsigned int n_seconds) = 0;

protected:
	c_wait_object(){};
};

class c_semaphore : public c_wait_object
{
public:
	c_semaphore(unsigned int n_init_hold = 0, unsigned int ulNameKey = 0, bool b_open = false);
	~c_semaphore();

public:
	int  set();
	int  reset();

public:
	int  check();
	int  wait(unsigned int n_seconds = -1);

private:
#ifdef _POSIX_
	sem_t			m_semaphore;
#else
    unsigned int get_suit_key();
    int             m_semaphore;
/////////////////////////////////
//  pthread_cond_t cond;
//  pthread_mutex_t mutex;
//  int counter;
//    mycondition_t m_Condition;
//    mymutex_t m_Mutex;
#endif
	unsigned int	m_n_hold;
};


// Create with Name
class c_Process_Event : public c_wait_object
{
public:
    c_Process_Event();
	~c_Process_Event();

public:
    int  Create_Event(unsigned int ulNameKey = 0, unsigned int n_init_hold = 0, bool b_open = false);
	int  set();
	int  reset();

public:
	int  check();
	int  wait(unsigned int n_seconds = -1);

private:
#ifdef _POSIX_
	sem_t             m_semaphore;
#else
    int             m_semaphore;
#endif
	unsigned int	m_ulNameKey;
    unsigned int	m_n_hold;
};

class c_Process_Mutex
{
public:
	c_Process_Mutex();
	~c_Process_Mutex();

public:
    unsigned int  Create_Mutex(unsigned int ulNameKey = 0);
    int  Lock();
	int  UnLock();

private:
    c_Process_Event m_Process_Event;
};


#define _nonsig_ 0
#define _sig_    1
/* Type of the mutex_cond object */
typedef struct mutex_cond{
        pthread_mutex_t i_mutex;
        pthread_cond_t i_cv;
		void* i_sigevent; // when "OR" flag i_sigevent = set event
}mutex_cond_t;
/* Type of the List element object */
typedef struct List_element{
        struct List_element *next;
        struct List_element *prev;
        struct mutex_cond *i_mutex_cond;
}List_element_t;
/* Type of the List object, Any other implementation
 * of List can be used
 */
typedef struct List{
        struct List_element *start_list;
        struct List_element *end_list;
        /* This mutex is used to protect the List critical section */
        pthread_mutex_t i_mutex;        
                                                                                      
}List_t;
/* Type of the Event Handle object */
typedef struct win32_event {
		int i_manual_reset;   /// 0 when thread wait return auto reset event non signal,1 mannual
        int i_signalled;
        struct List *i_list;
}simulate_win32_event_t;

int   clockgettime(struct timespec *tp, int msTimeOut);
void  addtolist( simulate_win32_event_t *event, List_element_t *new_element);
void  deletefromlist( simulate_win32_event_t *event, List_element_t *element );

typedef simulate_win32_event_t* GETSEV;
class c_event : public c_wait_object
{
public:
	c_event(bool b_manual_reset = false);
	~c_event();

public:
	int  set();
	int  reset();

public:
	int  check()
    {
        int ret = wait_success;

        pthread_mutex_lock(&m_sev.i_list->i_mutex);
        if(m_sev.i_signalled == _sig_)
        {
            if(m_sev.i_manual_reset != 1)
            {
                m_sev.i_signalled = 0; // reset signal
            }
        }
        else
        {
            ret = wait_error;
        }
        pthread_mutex_unlock(&m_sev.i_list->i_mutex);

        return ret;
    }

	int  wait(unsigned int n_seconds);

	bool  set_mode(bool b_manual_reset);
    operator GETSEV()
    {
        return &m_sev;
    }

    simulate_win32_event_t * getev()
    {
        return &m_sev;
    }
private:
//#ifdef _POSIX_
	//pthread_mutex_t m_mutex;
	//pthread_cond_t  m_condition;
//#else
	c_lock 			m_lock;
	int 			m_nWaitThrdNum; // call wait thread num

    simulate_win32_event_t m_sev;
	//c_semaphore     m_sem_event;
//#endif
	bool            m_b_manual_reset;
};

class c_wait_for
{
public:
	virtual ~c_wait_for(){};

public:
	virtual int wait_for_init(unsigned int n_count, c_wait_object* wait_object[]) = 0;
	virtual int wait_for_uninit() = 0;

public:
	virtual int wait_for_objects(unsigned int n_seconds = infinite, bool b_wait_all = false) = 0;

protected:
	c_wait_for();

	void add_reference();
	void dec_reference();
	unsigned int get_reference();

	c_wait_object** m_wait_object;
	unsigned int m_n_count;

private:
	unsigned int m_n_reference_count;
};

class c_wait_for_multiple_objects : public c_wait_for
{
public:
	int wait_for_init(unsigned int n_count, c_wait_object* wait_object[]);
	int wait_for_uninit();

public:
	int wait_for_objects(unsigned int n_seconds = infinite, bool b_wait_all = false);

private:
	void          wait_thread_proc();
	static void*  wait_thread(void* lp_param);
private:
	vector<pthread_t>			  m_v_wait_thread_pool;
	vector<bool>				  m_v_wait_object_state;
	map<pthread_t, unsigned int> m_m_wait_thread_id;
	c_lock						  m_wait_lock;
	c_lock                        m_wait_thread_lock;
	c_event						  m_wait_object_event;
	c_semaphore*				  m_p_wait_object_sem;
	int							  m_signal_flag;
	c_event                       m_start_timer_event;
	unsigned int                 m_current_timer_timeout;
	bool						  m_b_exit;
};

class c_wait_for_single_object : public c_wait_for
{
public:
	int wait_for_init(unsigned int n_count, c_wait_object* wait_object[]);
	int wait_for_uninit();

public:
	int wait_for_objects(unsigned int n_seconds = infinite, bool b_wait_all = false);
};

#endif //not _win32

#endif
