#ifndef __YD_SEM_H__
#define __YD_SEM_H__

#include <assert.h>

/* Semaphores */

#define __SEM_ID_NONE  0x0
#define __SEM_ID_LOCAL 0xcafef00d


#define sem_wait_x(s)			while (0 != sem_wait(s));

#ifdef FI_LINUX
#include <semaphore.h>
#endif //FI_LINUX

#if (__FreeBSD__ >= 10)

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>

/* Semaphores */

struct __local_sem_t
{
    unsigned int    count;
    pthread_mutex_t count_lock;
    pthread_cond_t  count_cond;
};

typedef struct yd_sem {
    int id;
    union {
        struct __local_sem_t local;
    } __data;
} yd_sem_t;

#define YD_SEM_VALUE_MAX ((int32_t)32767)

/* Caller must not include <semaphore.h> */

typedef yd_sem_t sem_t;

#define sem_init(s, p, v)   yd_sem_init(s, p, v)
#define sem_destroy(s)      yd_sem_destroy(s)
#define sem_getvalue(s, v)  yd_sem_getvalue(s, v)
#define sem_post(s)         yd_sem_post(s)
#define sem_timedwait(s, t) yd_sem_timedwait(s, t)
#define sem_trywait(s)      yd_sem_trywait(s)
#define sem_wait(s)         yd_sem_wait(s)

#define SEM_VALUE_MAX       YD_SEM_VALUE_MAX

static inline int yd_sem_init(yd_sem_t *sem, int pshared, unsigned int value)
{
	if (pshared) {
        errno = ENOSYS;
        return -1;
    }
  
    sem->id = __SEM_ID_NONE;

    if (pthread_cond_init(&sem->__data.local.count_cond, NULL)) {
        goto cond_init_fail;
    }

    if (pthread_mutex_init(&sem->__data.local.count_lock, NULL)) {
        goto mutex_init_fail;
    }

    sem->__data.local.count = value;
    sem->id = __SEM_ID_LOCAL;

    return 0;

mutex_init_fail:

    pthread_cond_destroy(&sem->__data.local.count_cond);

cond_init_fail:

    return -1;
}

static inline int yd_sem_destroy(yd_sem_t *sem)
{
	int res = 0;

    pthread_mutex_lock(&sem->__data.local.count_lock);

    sem->id = __SEM_ID_NONE;
    pthread_cond_broadcast(&sem->__data.local.count_cond);

    if (pthread_cond_destroy(&sem->__data.local.count_cond)) {
        res = -1;
    }

    pthread_mutex_unlock(&sem->__data.local.count_lock);

    if (pthread_mutex_destroy(&sem->__data.local.count_lock)) {
        res = -1;
    }

    return res;
}

static inline int yd_sem_getvalue(yd_sem_t *sem, unsigned int *sval)
{
	int res = 0;

    pthread_mutex_lock(&sem->__data.local.count_lock);

    if (sem->id != __SEM_ID_LOCAL) {
        res = -1;
        errno = EINVAL;
    } else {
        *sval = sem->__data.local.count;
    }

    pthread_mutex_unlock(&sem->__data.local.count_lock);

    return res;
}

static inline int yd_sem_post(yd_sem_t *sem)
{
	int res = 0;

    pthread_mutex_lock(&sem->__data.local.count_lock);

    if (sem->id != __SEM_ID_LOCAL) {
        res = -1;
        errno = EINVAL;
    } else if (sem->__data.local.count < YD_SEM_VALUE_MAX) {
        sem->__data.local.count++;
	if (sem->__data.local.count == 1) {
            pthread_cond_signal(&sem->__data.local.count_cond);
        }
    } else {
        errno = ERANGE;
        res = -1;
    }

    pthread_mutex_unlock(&sem->__data.local.count_lock);

    return res;
}

static inline int yd_sem_timedwait(yd_sem_t *sem, const struct timespec *abs_timeout)
{
	int res = 0;

    if (abs_timeout &&
        (abs_timeout->tv_nsec < 0 || abs_timeout->tv_nsec >= 1000000000)) {
       errno = EINVAL;
       return -1;
    }

    pthread_cleanup_push((void(*)(void*))&pthread_mutex_unlock,
                         &sem->__data.local.count_lock);

    pthread_mutex_lock(&sem->__data.local.count_lock);

    if (sem->id != __SEM_ID_LOCAL) {
        errno = EINVAL;
        res = -1;
    } else {
        if (!sem->__data.local.count) {
            res = pthread_cond_timedwait(&sem->__data.local.count_cond,
                                         &sem->__data.local.count_lock,
                                         abs_timeout);
        }
        if (res) {
            assert(res == ETIMEDOUT);
            res = -1;
            errno = ETIMEDOUT;
        } else if (sem->id != __SEM_ID_LOCAL) {
	    res = -1;
            errno = EINVAL;
	} else {
            sem->__data.local.count--;
        }
    }

    pthread_cleanup_pop(1);

    return res;
}

static inline int yd_sem_trywait(yd_sem_t *sem)
{
	int res = 0;

    pthread_mutex_lock(&sem->__data.local.count_lock);

    if (sem->id != __SEM_ID_LOCAL) {
        res = -1;
        errno = EINVAL;
    } else if (sem->__data.local.count) {
        sem->__data.local.count--;
    } else {
        res = -1;
        errno = EAGAIN;
    }

    pthread_mutex_unlock (&sem->__data.local.count_lock);

    return res;
}

static inline int yd_sem_wait(yd_sem_t *sem)
{
	int res = 0;

    pthread_cleanup_push((void(*)(void*))&pthread_mutex_unlock,
                          &sem->__data.local.count_lock);

    pthread_mutex_lock(&sem->__data.local.count_lock);

    if (sem->id != __SEM_ID_LOCAL) {
        errno = EINVAL;
        res = -1;
    } else {
        while (!sem->__data.local.count) {
            pthread_cond_wait(&sem->__data.local.count_cond,
                              &sem->__data.local.count_lock);
        }
        if (sem->id != __SEM_ID_LOCAL) {
	    res = -1;
            errno = EINVAL;
	} else {
            sem->__data.local.count--;
        }
    }

    pthread_cleanup_pop(1);

    return res;
}

#ifdef __cplusplus
}
#endif

#endif //__FreeBSD__=10
#endif //__YD_SEM_H__
