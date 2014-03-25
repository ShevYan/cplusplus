#include "c_wait_for.h"
#include "FiLog.h"
unsigned int system_v_semaphore_key = system_v_semaphore_key_begin;
c_lock system_v_semaphore_key_lock;

int semop_ex(int semaphore, struct sembuf* sem)
{
	int n_result = 0;

	while( 0 != ( n_result = semop(semaphore, sem,1)) &&
               EINTR == errno )
    {
        printf("[semop_ex] wait [%d]\n",semaphore);
    }

	return n_result;
}

/* c_lock implement part */

c_lock::c_lock()
{
	pthread_mutexattr_t attr,attr2;
	memset(&attr, 0, sizeof(pthread_mutexattr_t));
        memset(&attr2, 0, sizeof(pthread_mutexattr_t));

	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutexattr_settype(&attr2, PTHREAD_MUTEX_RECURSIVE);

	if (0 != pthread_mutex_init(&m_mutex , &attr))
	{
		printf("c_lock::c_lock pthread_mutex_init error<%d>.\n", errno);
		ASSERT(0);
	}

	if (0 != pthread_mutex_init(&m_self_mutex, &attr2))
	{
		printf("c_lock::c_lock pthread_mutex_init error<%d>.\n", errno);
		ASSERT(0);
	}

	m_owner_thread_id = 0;
	m_n_reference_count = 0;
}

c_lock::~c_lock()
{
	if(0 != pthread_mutex_destroy(&m_mutex))
	{
		printf("c_lock::~c_lock pthread_mutex_destroy error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}

	if(0 != pthread_mutex_destroy(&m_self_mutex))
	{
		printf("c_lock::~c_lock pthread_mutex_destroy error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
}

void
c_lock::lock()
{
    int r = 0;
	pthread_t current_thread_id = pthread_self();

	if(0 != pthread_mutex_lock(&m_mutex))
	{
		ASSERT("c_lock::lock pthread_mutex_lock " && 0);
	}

	if(0 != pthread_mutex_lock(&m_self_mutex))
	{
		printf("c_lock::lock pthread_mutex_lock error<%d>.\n", errno);
		ASSERT(0);
	}

	if(0 == m_n_reference_count)
	{
		m_n_reference_count++;
		/* we now own it for the first time.  Set owner information */
		m_owner_thread_id = current_thread_id;
	}

	if(0 != pthread_mutex_unlock(&m_self_mutex))
	{
		printf("c_lock::unlock pthread_mutex_unlock error<%d>.\n", errno);
		ASSERT(0);
	}

}

void
c_lock::unlock()
{
	pthread_t current_thread_id = pthread_self();

	if(0 != pthread_mutex_lock(&m_self_mutex))
	{
		printf("c_lock::lock pthread_mutex_lock error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}

	if (m_n_reference_count > 0) m_n_reference_count--;

	

	if(0 == m_n_reference_count)
	{
		m_owner_thread_id = 0;
	}

	if(0 != pthread_mutex_unlock(&m_self_mutex))
	{
		printf("c_lock::unlock pthread_mutex_unlock error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}

	if(0 != pthread_mutex_unlock(&m_mutex))
	{
		printf("c_lock::unlock pthread_mutex_unlock error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
}

pthread_t
c_lock::get_current_owner()
{
	return m_owner_thread_id;
}

#ifdef _POSIX_
#else
/* c_semaphore implement part */
//unsigned int c_semaphore::get_suit_key()
//{
//    unsigned int suit_semaphore_key = 0;
//
//    system_v_semaphore_key_lock.lock();
//    suit_semaphore_key = system_v_semaphore_key++;
//    system_v_semaphore_key_lock.unlock();
//
//    return suit_semaphore_key;
//}
#endif
c_semaphore::c_semaphore(unsigned int n_init_hold, unsigned int ulNameKey, bool b_open) : c_wait_object()
{
	m_n_hold = n_init_hold;

#ifdef _POSIX_
	if (0 != sem_init(&m_semaphore, 0, m_n_hold))
	{
		STRACE("c_semaphore::c_semaphore sem_init error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#else
    unsigned int suit_semaphore_key = 0;

    do
    {
        if (ulNameKey == 0)
        {
            suit_semaphore_key = IPC_PRIVATE;
        }
        else
        {
            suit_semaphore_key = ulNameKey;
        }

        /* check current key has used */
        m_semaphore = semget(suit_semaphore_key, 0, 0);

        if (-1 == m_semaphore && !b_open)
        {
            m_semaphore = semget(suit_semaphore_key, 1, IPC_CREAT|IPC_EXCL|0666);

            if (-1 == m_semaphore)
            {
                STRACE("c_semaphore::c_semaphore semget error<%d>.\n", errno);
                ASSERT(0);
                //exit(-1);
            }
            else
            {
                if (-1 == semctl(m_semaphore, 0, SETVAL, m_n_hold))
                {
                    STRACE("c_semaphore::c_semaphore semctl error<%d>.\n", errno);
                    ASSERT(0);
                    //exit(-1);
                }


                break;
            }
        }
        else if (b_open)
        {
            break;
        }

    } while (true);

    //printf("[semaphore open] %d\n", m_semaphore);
#endif
}

c_semaphore::~c_semaphore()
{
#ifdef _POSIX_
	if(0 != sem_destroy(&m_semaphore))
	{
		STRACE("c_semaphore::~c_semaphore sem_destroy error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#else
    union semun sem;

    sem.val = 0;

    //printf("\n[semop] delete [%d]\n",m_semaphore);
    if (-1 == semctl(m_semaphore, 0, IPC_RMID, sem))
    {
        STRACE("c_semaphore::~c_semaphore semctl error<%d>.\n", errno);
        ASSERT(0);
        //exit(-1);
    }

#endif
}

int
c_semaphore::set()
{
#ifdef _POSIX_
	if (0 != sem_post(&m_semaphore))
	{
		return false;
	}
#else
    struct sembuf sem;

    sem.sem_num = 0;
    sem.sem_op  = 1;
    sem.sem_flg = IPC_NOWAIT;

    if (-1 == semop_ex(m_semaphore, &sem))
    {
        STRACE("c_semaphore::set semop error<%d>.\n", errno);
        ASSERT(0);
        //exit(-1);
    }

#endif

	return true;
}

int
c_semaphore::reset()
{
#ifdef _POSIX_
//  if(0 != sem_destroy(&m_semaphore))
//  {
//  	return false;
//  }
//
//  if (0 != sem_init(&m_semaphore, 0, m_n_hold))
//  {
//  	return false;
//  }
	int nret = 0;
	do
	{
		nret = wait(0);
	} while (nret == 0);

	return true;
#else
    if (-1 == semctl(m_semaphore, 0, SETVAL, m_n_hold))
    {
        STRACE("c_semaphore::reset semctl error<%d>.\n", errno);
        ASSERT(0);
        //exit(-1);
    }

#endif

	return true;
}

int c_semaphore::check()
{
    return wait(0);
}

int
c_semaphore::wait(unsigned int n_seconds)
{
	int n_result = wait_success;
	int n_tmp_result = wait_success;
	int n_wait_time = 0;

#ifdef _POSIX_
	if(infinite == n_seconds)
	{
        while( 0 != ( n_result = sem_wait( &m_semaphore ) ) &&
			EINTR == errno )
		{
			continue;
		}
	}
	else
	{
       struct timespec ts;
       struct timeval tt;
       struct timezone tz;
       int sec,usec;
       gettimeofday(&tt,&tz);
       sec = n_seconds /1000;
       n_seconds = n_seconds - (sec*1000);
       assert(n_seconds < 1000);
       usec = n_seconds *1000;
       assert(tt.tv_usec < 1000000);


       ts.tv_sec = tt.tv_sec + sec;
       ts.tv_nsec = (tt.tv_usec + usec) * 1000;
       assert(ts.tv_nsec < 2000000000);
       if(ts.tv_nsec > 999999999)
       {
           ts.tv_sec++;
           ts.tv_nsec -= 1000000000;
       }
    
        while( 0 != ( n_result =sem_timedwait(&m_semaphore,&ts) ) &&
            EINTR == errno )
        {
            continue;
        }

        if (n_result == -1)
        {
               if (errno == ETIMEDOUT)
               {
                   //printf("sem_timedwait() timed out\n");
                   n_result = wait_timeout;
               }
               else
               {
                   //perror("sem_timedwait");
                   n_result = wait_error;
               }
        }
        else
        {
            n_result = wait_success;
        }

/*		struct timespec delay_time;
		delay_time.tv_sec = 0;
		delay_time.tv_nsec = 1000000;	//1ms


		while (true)
		{
			n_tmp_result = sem_trywait(&m_semaphore);

			if (0 == n_tmp_result)
			{
				break;
			}

			if (EAGAIN  != errno)
			{
				n_result = wait_error;

				break;
			}

			if (n_seconds <= n_wait_time)
			{
				n_result = wait_timeout;

				break;
			}

			nanosleep(&delay_time, NULL);

			n_wait_time++;
		}*/
	}
#else
    struct sembuf sem;

    sem.sem_num = 0;
    sem.sem_op  = -1;

    if(infinite == n_seconds)
    {
        sem.sem_flg =0; //SEM_UNDO;

        n_tmp_result = semop_ex(m_semaphore, &sem);

        if (0 != n_tmp_result)
        {
            n_result = errno;

            STRACE("mac event wait infinite error[%d]",n_result);
            ASSERT(0);
        }
    }
    else
    {
        sem.sem_flg = IPC_NOWAIT;


        struct timespec delay_time;

        delay_time.tv_sec = 0;
        delay_time.tv_nsec = 1000000;	//1ms

        while (true)
        {
            n_tmp_result = semop_ex(m_semaphore, &sem);

            if (0 == n_tmp_result)
            {
                break;
            }

            /* n_tmp_result == -1 */
            if (EAGAIN  != errno)
            {
                n_result = wait_error;
                STRACE("c_semaphore::wait semop return Error<%d>.\n", errno);
                break;
            }

            if (n_seconds <= n_wait_time)
            {
                n_result = wait_timeout;

                break;
            }

            nanosleep(&delay_time, NULL);

            n_wait_time++;
        }
    }

    #endif

	return n_result;
}

/* c_event implement part */
c_event::c_event(bool b_manual_reset) : c_wait_object()
{
    m_nWaitThrdNum = 0;
	m_b_manual_reset = b_manual_reset;
    m_sev.i_manual_reset = b_manual_reset;
    m_sev.i_signalled = _nonsig_;
	m_sev.i_list = (struct List *)malloc(sizeof(struct List));		
	m_sev.i_list->start_list = (List_element*)NULL;
	m_sev.i_list->end_list = (List_element*)NULL;

	pthread_mutex_init(&(m_sev.i_list->i_mutex), NULL);
	
}

c_event::~c_event()
{
    pthread_mutex_destroy(&m_sev.i_list->i_mutex);
    free(m_sev.i_list);
}

void addtolist( simulate_win32_event_t *event, List_element_t *new_element)
{
	int ret = -1;
	struct List_element *start, *end;
	struct List_element *p = (struct List_element*)new_element ;

	ret = pthread_mutex_lock(&(event->i_list->i_mutex));
	assert(ret == 0);

	start = event->i_list->start_list;
	end = event->i_list->end_list;

	if ( start == NULL)
	{
		/* First element of the list */
		start = p;
		p->next = (List_element*)NULL;
		p->prev = (List_element*)NULL;
		end = p;
		event->i_list->start_list = p;
		event->i_list->end_list = p;
	}
	else
	{
		/* 
		 * Already some elements in the list...
		 * add this element at the end 
		 */
		end->next = p;
		p->prev = end;
		p->next =(List_element*) NULL;
		end = p;
		event->i_list->end_list = p;
	}

	ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
	assert(ret == 0);

} /* addtolist */

/*
 * Delete element from the list of mutex_cond_t...
 */
void deletefromlist( simulate_win32_event_t *event, List_element_t *element )
{
	int ret = -1;
	struct List_element *start, *end, *q;
	struct List_element *p = (struct List_element *)element ;

//  ret = pthread_mutex_lock(&(event->i_list->i_mutex));
//  assert(ret == 0);

	start = event->i_list->start_list;
	end = event->i_list->end_list;

	if ( start == p && end == p )
	{
		/* only element in the list */ 
		start = end = (List_element*)NULL;
		/*
		 * Call the appropriate function for freeing
		 * the memory
		 */
		free(p);
		event->i_list->start_list = (List_element*)NULL;
		event->i_list->end_list = (List_element*)NULL;
	}
	else if ( start == p )
	{
		q = p->next;
		start = q;
		q->prev =(List_element*) NULL;        
		free(p);
		event->i_list->start_list = start;
	}
	else if ( end == p )
	{
		end = p->prev;  
		end->next = (List_element*)NULL;
		free(p);
		event->i_list->end_list = end;
	}
	else
	{
		/*
		 * Element is somehwre in the middle of the list 
		 */
		(p->prev)->next = p->next;
		(p->next)->prev = p->prev;
		free(p);
	}

//  ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
//  assert(ret == 0);
} /* deletefromlist */


//int
//c_event::set()
//{
//	unsigned int n_result = wait_success;
//	int nNum = 0;
//	int i = 0;
//
//	m_lock.lock();
//	nNum = m_nWaitThrdNum;
//	m_lock.unlock();
//
//    simulate_win32_event_t *event = &m_sev;
//	struct List_element *start = NULL;
//	struct List_element *p = NULL;
//	int ret = -1;
//	/*
//	 * At this point iterate through the list
//	 */
//	ret = pthread_mutex_lock(&(event->i_list->i_mutex));
//	assert(ret == 0);
//
//	event->i_signalled = _sig_;
//
//	start = event->i_list->start_list;
//	if ( start == NULL )
//	{
//		//fprintf(stderr, " event[%x]Nothing to iterate...no elements in the list!\n",&m_sev);
//		ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
//		assert(ret == 0);
//
//		return wait_success;
//	}
//	/*
//	 * All the mutexes on the Event Handle List are first
//	 * locked, then all condition variables are signalled
//	 * and then all mutexes are unlocked. This is done to
//	 * prevent any race condition.
//	 */
//	p = start;
//	do
//	{
//		ret = pthread_mutex_lock( &(p->i_mutex_cond->i_mutex) );
//        assert(ret == 0);
//		p->i_mutex_cond->i_sigevent = event;
//		p = p->next;
//	} while ( p != NULL );
//
//	p = start;
//
//	do
//	{
//		pthread_cond_signal( &(p->i_mutex_cond->i_cv) ); 
//		//printf("signal condition [%d]\n",p->i_mutex_cond->i_cv.__sig);
//		p = p->next;
//	} while ( p != NULL );
//
//	p = start;
//
//	do
//	{
//		ret = pthread_mutex_unlock( &(p->i_mutex_cond->i_mutex) ); 
//        assert(ret == 0);
//		p = p->next;
//	} while ( p != NULL );
//
//	ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
//	assert(ret == 0);
//
//	return n_result;    
//}

int
c_event::set()
{
	unsigned int n_result = wait_success;
	int nNum = 0;
	int i = 0;

	m_lock.lock();
	nNum = m_nWaitThrdNum;
	m_lock.unlock();

    simulate_win32_event_t *event = &m_sev;
	struct List_element *start = NULL;
	struct List_element *p = NULL;
	int ret = -1;
	/*
	 * At this point iterate through the list
	 */
	ret = pthread_mutex_lock(&(event->i_list->i_mutex));
	assert(ret == 0);

	event->i_signalled = _sig_;

	start = event->i_list->start_list;
	if ( start == NULL )
	{
		//fprintf(stderr, " event[%x]Nothing to iterate...no elements in the list!\n",&m_sev);
		ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
		assert(ret == 0);

		return wait_success;
	}
	/*
	 * All the mutexes on the Event Handle List are first
	 * locked, then all condition variables are signalled
	 * and then all mutexes are unlocked. This is done to
	 * prevent any race condition.
	 */
	p = start;
	do
	{
		ret = pthread_mutex_lock( &(p->i_mutex_cond->i_mutex) );
        assert(ret == 0);
		p->i_mutex_cond->i_sigevent = event;

		pthread_cond_signal( &(p->i_mutex_cond->i_cv) ); 
		//printf("signal condition [%d]\n",p->i_mutex_cond->i_cv.__sig);
		ret = pthread_mutex_unlock( &(p->i_mutex_cond->i_mutex) ); 
        assert(ret == 0);
		p = p->next;
	} while ( p != NULL );

	ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
	assert(ret == 0);

	return n_result;    
}

int
c_event::reset()
{
	unsigned int n_result = wait_success;
    simulate_win32_event_t *event = &m_sev;
	int ret = -1;
	/*
	 * At this point iterate through the list
	 */
	ret = pthread_mutex_lock(&(event->i_list->i_mutex));
	assert(ret == 0);
	event->i_signalled = _nonsig_;
	ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
	assert(ret == 0);

	return n_result;
}

int clockgettime(struct timespec *tp, int msTimeOut)
{
	struct timeval tt;
	struct timezone tz;
	int sec,usec;

	gettimeofday(&tt,&tz);

	sec = msTimeOut /1000;
	msTimeOut = msTimeOut - (sec*1000);
	assert(msTimeOut < 1000);
	usec = msTimeOut *1000;
	assert(tt.tv_usec < 1000000);
	tp->tv_sec = tt.tv_sec + sec;
	tp->tv_nsec = (tt.tv_usec + usec) * 1000;
	assert(tp->tv_nsec < 2000000000);

	if (tp->tv_nsec > 999999999)
	{
		tp->tv_sec++;
		tp->tv_nsec -= 1000000000;
	}

	return(1);
}

int
c_event::wait(unsigned int  n_seconds)
{
	int n_result = wait_success;

	m_lock.lock();
	m_nWaitThrdNum++;
	m_lock.unlock();

    simulate_win32_event_t *event;
	int Result = wait_success;
	int i;
	int ret = -1;
	struct timespec to_time; /* time value */
	struct timespec delta_time;	/* time value */
	mutex_cond_t *p = (mutex_cond_t*)NULL;
	mutex_cond_t *common_mutex_cond = (mutex_cond_t*)NULL;
	List_element_t *q = (List_element_t*)NULL;
	/*
	 * Local array of mutex_cond objects and List is maintained
	 * so that a search through the Event list is not required
	 * later when these objects are needed.
	 */
	mutex_cond_t *mutex_cond = (mutex_cond_t*)NULL;
	List_element_t *list_element = (List_element_t*)NULL;

	if (n_seconds != -1)
	{
        clockgettime(&to_time, n_seconds);
	}

    event = &m_sev;
    if (event == NULL)
    {
        printf("============wait return error\n");
        Result = wait_error;
        goto LABLE;
    }

    Result = wait_success;
    /*
     * mutex_cond objects are created and initialized, List
     * object elements are created, and mutex_cond object is
     * is added to each List element.
     */

    p = (mutex_cond_t *) malloc(sizeof(mutex_cond_t));
    p->i_sigevent = NULL;
    ret = pthread_mutex_init ( &(p->i_mutex), NULL);
    assert(ret == 0);
    ret = pthread_cond_init ( &(p->i_cv), NULL);
    assert(ret == 0);
    q = (List_element_t *) malloc(sizeof(List_element_t));
    mutex_cond = p;
    q->i_mutex_cond = p;
    list_element = q;
    /*
     * At this point we need to add the malloced
     * object to the list
     */

    addtolist( event, q);

/* Wait on each event to be set or timedout */


    ret = pthread_mutex_lock(&(mutex_cond->i_mutex));
    assert(ret == 0);

    if (event->i_signalled != _sig_)
    {
        if (n_seconds != -1)
        {
            if (pthread_cond_timedwait(&(mutex_cond->i_cv),
                                       &(mutex_cond->i_mutex), &to_time) ==ETIMEDOUT)
            {
                Result = wait_timeout;
            }
        }
        else
        {
            ret = pthread_cond_wait(&(mutex_cond->i_cv),&(mutex_cond->i_mutex));
            Result = wait_success; // AND return wait success
        }
    }

    ret = pthread_mutex_unlock(&(mutex_cond->i_mutex));
    assert(ret == 0);

    /*               
     * Delete all the List elements created by this thread for
     * each event. and delete temp mutex_cond
     */


    ret = pthread_mutex_lock(&event->i_list->i_mutex);
	assert(ret == 0);

    if(event->i_manual_reset == 0)
    {
        event->i_signalled = _nonsig_; // all wait thread exit reset nosingle  
    }

    deletefromlist(event, list_element);

    pthread_mutex_destroy(&(mutex_cond->i_mutex));
    pthread_cond_destroy(&(mutex_cond->i_cv));

    free(mutex_cond);
    mutex_cond = NULL;
    p = NULL;


    ret = pthread_mutex_unlock(&event->i_list->i_mutex);
	assert(ret == 0);

LABLE:
	m_lock.lock();
	m_nWaitThrdNum--;
	m_lock.unlock();

	return Result;
}

bool c_event::set_mode(bool b_manual_reset)
{
    m_b_manual_reset = b_manual_reset;
    m_sev.i_manual_reset = m_b_manual_reset;
}

/* c_wait_for implement part */
c_wait_for::c_wait_for() : m_n_reference_count(0)
{
}

void
c_wait_for::add_reference()
{
	m_n_reference_count++;
}

void
c_wait_for::dec_reference()
{
	if (m_n_reference_count == 0)
	{
		STRACE("c_wait_for::dec_reference m_n_reference_count exception.\n");
		ASSERT(0);
		//exit(1);
	}

	m_n_reference_count--;
}

unsigned int
c_wait_for::get_reference()
{
	return m_n_reference_count;
}

/* c_wait_for_multiple_objects implement part */
int
c_wait_for_multiple_objects::wait_for_init(unsigned int n_count, c_wait_object* wait_object[])
{
	pthread_t tmp_thread_id = 0;
	unsigned int n_index = 0;

	if (n_count <= 0 || wait_object == NULL || wait_object[0] == NULL)
	{
		return invalid_wait_object;
	}

	m_b_exit = false;

	/* confirm first waiting thread can setting signal_flag */
	m_p_wait_object_sem = new c_semaphore(1);
	check_new_valid(m_p_wait_object_sem);

	m_n_count = n_count;
	m_current_timer_timeout = 0;

	m_wait_object = new c_wait_object*[m_n_count];
	check_new_valid(m_wait_object);

	memcpy(m_wait_object, wait_object, m_n_count*sizeof(c_wait_object*));

	m_wait_thread_lock.lock();

	/* one timer thread */
	for (n_index = 0; n_index < m_n_count+1; n_index++)
	{
		if(0 != pthread_create(&tmp_thread_id, NULL, wait_thread, (void *)this))
		{
			wait_for_uninit();

			m_wait_thread_lock.unlock();

			return wait_error;
		}

		m_v_wait_thread_pool.push_back(tmp_thread_id);
		m_m_wait_thread_id.insert(map<pthread_t, unsigned int>::value_type(tmp_thread_id, n_index));

		if (n_index != m_n_count)
		{
			m_v_wait_object_state.push_back(false);
		}
	}

	m_wait_thread_lock.unlock();

	return wait_success;
}

int
c_wait_for_multiple_objects::wait_for_uninit()
{
	unsigned int n_index = 0;

	m_b_exit = true;

	for (n_index = 0; n_index < m_v_wait_thread_pool.size(); n_index++)
	{
		if (n_index != m_v_wait_thread_pool.size()-1)
		{
			m_wait_object[n_index]->set();
		}

		pthread_cancel(m_v_wait_thread_pool[n_index]);
		//pthread_join(m_v_wait_thread_pool[n_index] , NULL);
	}

	safe_delete_array(m_wait_object);
	safe_delete_pointer(m_p_wait_object_sem);

	return wait_success;
}

void
c_wait_for_multiple_objects::wait_thread_proc()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);		// ENABLE CANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	// NO DELAY EXIT

	m_wait_thread_lock.lock();

	unsigned int n_index = m_m_wait_thread_id[pthread_self()];

	m_wait_thread_lock.unlock();

	if (n_index > m_n_count || m_n_count == 0)
	{
		return;
	}

	if (n_index == m_n_count)
	{
		timeval wait_time_val;

		wait_time_val.tv_usec = 0;

		// timer
		while (!m_b_exit)
		{
			pthread_testcancel();

			m_start_timer_event.wait(infinite);

			pthread_testcancel();

			if (INFINITE == m_current_timer_timeout)
			{
				break;
			}

			wait_time_val.tv_sec = m_current_timer_timeout;

			select(0, NULL, NULL, NULL, &wait_time_val);

			pthread_testcancel();

			/* get setting signal_flag rigths */
			if (wait_timeout != m_p_wait_object_sem->check())
			{
				m_signal_flag = wait_timeout;
				m_wait_object_event.set();
			}
		}
	}
	else
	{
		while (!m_b_exit && m_wait_object[n_index] != NULL)
		{
			pthread_testcancel();

			if (wait_success == m_wait_object[n_index]->wait(infinite))
			{
				pthread_testcancel();

				/* get setting signal_flag rigths */
				m_p_wait_object_sem->wait(infinite);

				m_signal_flag = n_index;
				m_wait_object_event.set();
			}
			else
			{
				STRACE("wait_thread wait object exception, error<%d>.\n", errno);
				ASSERT(0);
				//exit(-1);
			}
		}
	}
}

void*
c_wait_for_multiple_objects::wait_thread(void* lp_param)
{
	(static_cast<c_wait_for_multiple_objects*>(lp_param))->wait_thread_proc();

	return NULL;
}

int
c_wait_for_multiple_objects::wait_for_objects(unsigned int n_seconds, bool b_wait_all)
{
	int n_result = 0;
	int n_index = 0;

	m_wait_lock.lock();

	m_current_timer_timeout = n_seconds;
	m_start_timer_event.set();

	do
	{
		n_result = m_wait_object_event.wait(infinite);

		if (wait_timeout != m_signal_flag)
		{
			if (!b_wait_all)
			{
				n_result = (int)m_signal_flag;

				m_p_wait_object_sem->set();

				break;
			}

			if (m_v_wait_object_state.size() < m_signal_flag)
			{
				STRACE("c_wait_for_multiple_objects::wait_for_objects m_signal_flag return(%d) exception.\n", m_signal_flag);

				ASSERT(0);
                //exit(-1);
			}

			m_v_wait_object_state[m_signal_flag] = true;

			/* allow one thread setting next signal_flag */
			m_p_wait_object_sem->set();

			for (n_index = 0; n_index < m_v_wait_object_state.size(); n_index++)
			{
				if (m_v_wait_object_state[n_index] == false)
				{
					break;
				}
			}

			if (n_index == m_v_wait_object_state.size())
			{
				n_result = wait_success;

				break;
			}
		}
		else
		{
			n_result = wait_timeout;

			break;
		}

	} while (b_wait_all);

	for (n_index = 0; n_index < m_v_wait_object_state.size(); n_index++)
	{
		m_v_wait_object_state[n_index] = false;
	}

	m_wait_lock.unlock();

	return n_result;
}

/* c_wait_for_single_object implement part */
 int
c_wait_for_single_object::wait_for_init(unsigned int n_count, c_wait_object* wait_object[])
{
	if (n_count <= 0 || wait_object == NULL || wait_object[0] == NULL)
	{
		return invalid_wait_object;
	}

	m_n_count = 1;

	m_wait_object = new c_wait_object*[m_n_count];
	check_new_valid(m_wait_object);

	m_wait_object[0] = wait_object[0];

	return wait_success;
}

int
c_wait_for_single_object::wait_for_uninit()
{
	safe_delete_array(m_wait_object);
	return wait_success;
}

int
c_wait_for_single_object::wait_for_objects(unsigned int n_seconds, bool b_wait_all)
{
	c_wait_object* p_object = m_wait_object[0];

	return p_object->wait(n_seconds) ;

}

c_Process_Mutex::c_Process_Mutex()
{
}

unsigned int  c_Process_Mutex::Create_Mutex(unsigned int ulNameKey )
{
	return m_Process_Event.Create_Event(ulNameKey, 1);
}

c_Process_Mutex::~c_Process_Mutex()
{
}

int  c_Process_Mutex::Lock()
{

	return m_Process_Event.wait();
}

int  c_Process_Mutex::UnLock()
{
	return m_Process_Event.set();
}

c_Process_Event::c_Process_Event()
{
}

int c_Process_Event::Create_Event (unsigned int ulNameKey, unsigned int n_init_hold, bool b_open)
{
	m_n_hold = n_init_hold;

#ifdef _POSIX_
	if (0 != sem_init(&m_semaphore, 0, m_n_hold))
	{
		STRACE("c_Process_Event::c_semaphore sem_init error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#else
	unsigned int suit_semaphore_key = 0;

	do {
		if (ulNameKey != 0)
		{
			suit_semaphore_key = ulNameKey;

			m_semaphore = semget(suit_semaphore_key, 0, 0);

			if (-1 == m_semaphore && !b_open)
			{
				m_semaphore = semget(suit_semaphore_key, 1, IPC_CREAT|IPC_EXCL|0666);

				if (-1 == m_semaphore)
				{
					STRACE("c_Process_Event::c_semaphore semget error<%d>.\n", errno);
                    ASSERT(0);
					//exit(-1);
				}
				else
				{
					if (-1 == semctl(m_semaphore, 0, SETVAL, m_n_hold))
					{
						STRACE("c_Process_Event::c_semaphore semctl error<%d>.\n", errno);
                        ASSERT(0);
						//exit(-1);
					}

					break;
				}
			}
			else
			{
				STRACE("c_Process_Event:: m_semaphore %d\n", m_semaphore);
				break;
			}

		}
		else
		{
			suit_semaphore_key = IPC_PRIVATE;//get_suit_key();
			/* check current key has used */
			m_semaphore = semget(suit_semaphore_key, 0, 0);

			if (-1 == m_semaphore)
			{
				m_semaphore = semget(suit_semaphore_key, 1, IPC_CREAT|IPC_EXCL|0666);

				if (-1 == m_semaphore)
				{
					STRACE("c_semaphore::c_semaphore semget error<%d>.\n", errno);
                    ASSERT(0);
                    //exit(-1);
				}
				else
				{
					if (-1 == semctl(m_semaphore, 0, SETVAL, m_n_hold))
					{
						STRACE("c_semaphore::c_semaphore semctl error<%d>.\n", errno);
                        ASSERT(0);
						//exit(-1);
					}

					break;
				}
			}
		}

	} while (true);

#endif
	return 1;
}

c_Process_Event::~c_Process_Event()
{
#ifdef _POSIX_
	if(0 != sem_destroy(&m_semaphore))
	{
		STRACE("c_semaphore::~c_semaphore sem_destroy error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#else
	union semun sem;

	sem.val = 0;
	//printf("\n[semop] delete [%d]\n",m_semaphore);
	if (-1 == semctl(m_semaphore, 0, IPC_RMID, sem))
	{
		STRACE("c_semaphore::~c_semaphore semctl error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#endif
}

int
c_Process_Event::set()
{
#ifdef _POSIX_
	if (0 != sem_post(&m_semaphore))
	{
		return false;
	}
#else
	struct sembuf sem;

	sem.sem_num = 0;
	sem.sem_op  = 1;
	sem.sem_flg = IPC_NOWAIT;

	if (-1 == semop_ex(m_semaphore, &sem))
	{
		STRACE("c_semaphore::set semop error<%d>.\n", errno);
		ASSERT(0);
		//exit(-1);
	}
#endif

	return true;
}

int
c_Process_Event::reset()
{
#ifdef _POSIX_
	if(0 != sem_destroy(&m_semaphore))
	{
		return false;
	}

	if (0 != sem_init(&m_semaphore, 0, m_n_hold))
	{
		return false;
	}
#else
	if (-1 == semctl(m_semaphore, 0, SETVAL, m_n_hold))
	{
		STRACE("c_semaphore::reset semctl error<%d>.\n", errno);
        ASSERT(0);
		//exit(-1);
	}
#endif

	return true;
}

int
c_Process_Event::check()
{
#ifdef _POSIX_
	if (0 != sem_trywait(&m_semaphore))
	{
		return false;
	}
#else
	return wait(0);
#endif

	//return true;
}

int
c_Process_Event::wait(unsigned int n_seconds)
{
	int n_result = wait_success;
	int n_tmp_result = wait_success;
	unsigned int n_wait_time = 0;

#ifdef _POSIX_
	if(infinite == n_seconds)
	{
        while( 0 != ( n_result = sem_wait( &m_semaphore ) ) &&
               EINTR == errno );

        if (0 != n_result)
		{
			n_result = wait_error;
		}
	}
	else
	{
        struct timespec ts;
       struct timeval tt;
       struct timezone tz;
       int sec,usec;
       gettimeofday(&tt,&tz);
       sec = n_seconds /1000;
       n_seconds = n_seconds - (sec*1000);
       assert(n_seconds < 1000);
       usec = n_seconds *1000;
       assert(tt.tv_usec < 1000000);


       ts.tv_sec = tt.tv_sec + sec;
       ts.tv_nsec = (tt.tv_usec + usec) * 1000;
       assert(ts.tv_nsec < 2000000000);
       if(ts.tv_nsec > 999999999)
       {
           ts.tv_sec++;
           ts.tv_nsec -= 1000000000;
       }
    
        while( 0 != ( n_result =sem_timedwait(&m_semaphore,&ts) ) &&
            EINTR == errno )
        {
            continue;
        }

        if (n_result == -1)
        {
               if (errno == ETIMEDOUT)
               {
                   //printf("sem_timedwait() timed out\n");
                   n_result = wait_timeout;
               }
               else
               {
                   //perror("sem_timedwait");
                   n_result = wait_error;
               }
        }
        else
        {
            n_result = wait_success;
        }
/*		struct timespec delay_time;

		delay_time.tv_sec = 0;
		delay_time.tv_nsec = 1000000;	//1ms

		while (true)
		{
			n_tmp_result = sem_trywait(&m_semaphore);

			if (0 == n_tmp_result)
			{
				break;
			}

			if (EAGAIN  != n_tmp_result)
			{
				n_result = wait_error;

				break;
			}

			if (n_seconds <= n_wait_time)
			{
				n_result = wait_timeout;

				break;
			}

			nanosleep(&delay_time, NULL);

			n_wait_time++;
		}*/
	}
#else
	struct sembuf sem;

	sem.sem_num = 0;
	sem.sem_op  = -1;

	if(infinite == n_seconds)
	{
		sem.sem_flg =0; //SEM_UNDO;

		n_tmp_result = semop_ex(m_semaphore, &sem);

		if (0 != n_tmp_result)
		{
			n_result = wait_error;
		}
	}
	else
	{
		sem.sem_flg = IPC_NOWAIT;//SEM_UNDO | IPC_NOWAIT;

		n_seconds = n_seconds;

		struct timespec delay_time;

		delay_time.tv_sec = 0;
		delay_time.tv_nsec = 1000000;	//1ms

		while (true)
		{
			n_tmp_result = semop_ex(m_semaphore, &sem);

			if (0 == n_tmp_result)
			{
				break;
			}

			/* n_tmp_result == -1 */
			if (EAGAIN  != errno)
			{
				n_result = wait_error;

				break;
			}

			if (n_seconds <= n_wait_time)
			{
				n_result = wait_timeout;

				break;
			}

			nanosleep(&delay_time, NULL);

			n_wait_time++;
		}
	}
#endif

	return n_result;
}
