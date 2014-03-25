#ifndef __OSWAITOBJEDT_H__
#define __OSWAITOBJEDT_H__

#pragma once

#include "OSEvent.h"
#include "OSMutex.h"
#include "OSSemaphore.h"


// Only for one.WaitMulObjects

inline int OSWaitMulObjects(DWORD Count, FiEvent** Events, BOOL fWaitall, unsigned int dwMilliseconds)
{
#if defined(_WIN32) || defined(_WIN64)
	vector<HANDLE> tmp;
	for(DWORD i =0; i < Count; i++)
	{
		tmp.push_back(Events[i]->GetHandle());
	}
	
	return WaitForMultipleObjects(Count, &tmp[0], fWaitall, dwMilliseconds);

#else
    int i , nret = 0;
    simulate_win32_event_t *event = NULL;
    int Result = wait_success;
    int ret = -1;
    struct timespec to_time; /* time value */
    struct timespec delta_time; /* time value */
    mutex_cond_t *p = (mutex_cond_t*)NULL;
    mutex_cond_t *common_mutex_cond = (mutex_cond_t*)NULL;
    List_element_t *q = (List_element_t*)NULL;
    /*
     * Local array of mutex_cond objects and List is maintained
     * so that a search through the Event list is not required
     * later when these objects are needed.
     */
    mutex_cond_t **mutex_cond = (mutex_cond_t**)NULL;
    List_element_t **list_element = (List_element_t**)NULL;

    if (fWaitall == TRUE)
    {
        mutex_cond = (mutex_cond_t **)malloc (sizeof(mutex_cond_t *)* Count);
    }

    list_element = (List_element_t **)malloc (sizeof(List_element_t *)* Count);

    if (dwMilliseconds != -1)
    {
        clockgettime(&to_time, dwMilliseconds);
    }

    for (i = 0; i < Count; i++)
    {
        event = (GETSEV)((WAITHANDLE)Events[i]);
        if (event == NULL)
        {
            printf("============wait return error\n");
            Result = wait_error;
            goto LABLE;
        }
    }

    if (fWaitall)
    {

        /*
         * mutex_cond objects are created and initialized, List
         * object elements are created, and mutex_cond object is
         * is added to each List element.
         */
        for (i=0; i < Count; i++)
        {
            event =Events[i]->GetHandle()->getev();
            p = (mutex_cond_t *) malloc(sizeof(mutex_cond_t));
            p->i_sigevent = NULL;
            ret = pthread_mutex_init ( &(p->i_mutex), NULL);
            assert(ret == 0);
            ret = pthread_cond_init ( &(p->i_cv), NULL);
            assert(ret == 0);
            q = (List_element_t *) malloc(sizeof(List_element_t));
            mutex_cond[i] = p;
            q->i_mutex_cond = p;
            list_element[i]=q;
            /*
             * At this point we need to add the malloced
             * object to the list
             */

            addtolist(event, q);
        }
/* Wait on each event to be set or timedout */

        for (i=0; i < Count; i++)
        {
            event = Events[i]->GetHandle()->getev();
            ret = pthread_mutex_lock(&(mutex_cond[i]->i_mutex));
            assert(ret == 0);

            while (event->i_signalled != _sig_)
            {
                if (dwMilliseconds != -1)
                {
                    if (pthread_cond_timedwait(&(mutex_cond[i]->i_cv),
                                               &(mutex_cond[i]->i_mutex), &to_time) ==ETIMEDOUT)
                    {
                        Result = wait_timeout;
                        break;
                    }
                }
                else
                {
                    pthread_cond_wait(&(mutex_cond[i]->i_cv),&(mutex_cond[i]->i_mutex));
                    Result = wait_success; // AND return wait success
                }
            }

            ret = pthread_mutex_unlock(&(mutex_cond[i]->i_mutex));
            assert(ret == 0);

            if (Result == wait_timeout)
            {
                break;
            }
        }
        /*               
         * Delete all the List elements created by this thread for
         * each event. and delete temp mutex_cond
         */
        for (i=0; i < Count; i++)
        {
            event = Events[i]->GetHandle()->getev();


            ret = pthread_mutex_lock(&event->i_list->i_mutex);
            assert(ret == 0);

            if (event->i_manual_reset != 1)
            {
                event->i_signalled = _nonsig_; // all wait thread exit reset nosingle  
            }

            deletefromlist(event, list_element[i]);

            pthread_mutex_destroy(&(mutex_cond[i]->i_mutex));
            pthread_cond_destroy(&(mutex_cond[i]->i_cv));

            free(mutex_cond[i]);       

            ret = pthread_mutex_unlock(&event->i_list->i_mutex);
            assert(ret == 0);
        }


    }
    else
    {
        Result = wait_error;
        /*
         * A common mutex_cond object is created and initialized, List
         * object elements are created, and same common mutex_cond
         * object is added to each List element.
         */
        common_mutex_cond = (mutex_cond_t *)malloc(sizeof(mutex_cond_t));
        common_mutex_cond->i_sigevent = NULL;

        pthread_mutex_init(&common_mutex_cond->i_mutex, NULL);
        pthread_cond_init(&common_mutex_cond->i_cv, NULL);

        for (i=0; i < Count; i++)
        {
            event = Events[i]->GetHandle()->getev();
            q = (List_element_t *) malloc(sizeof(List_element_t));
            q->i_mutex_cond = common_mutex_cond;
            list_element[i]=q;
            /*
             * At this point the malloced List element is
             * added to the Event Handle.
             */
            addtolist(event, q);
        }
        /*
         * It is first tested if any of the event has already
         * been set, if yes, the function returns as expected.
         * Or else, wait on the common mutex for any event to
         * be set or time out.
         */
        ret = pthread_mutex_lock(&common_mutex_cond->i_mutex);
        assert(ret == 0);

        for (i=0; i < Count; i++)
        {
            event = Events[i]->GetHandle()->getev();

            if (event->i_signalled == _sig_)
            {
                Result = i;
                break;
            }
        }

        if (Result == wait_error)
        {
            if (dwMilliseconds != -1)
            {
                if (pthread_cond_timedwait(&common_mutex_cond->i_cv, 
                                           &common_mutex_cond->i_mutex,
                                           &to_time) != ETIMEDOUT)
                {
                    for (i = 0; i < Count; i++)
                    {
                        if (common_mutex_cond->i_sigevent == Events[i]->GetHandle()->getev())
                        {
                            Result = i; // or return i obj
                            break;
                        }
                    }

                    if (i == Count)
                    { // not find common_mutex_cond->i_sigevent == Events[i]
                        assert(0);
                    }
                }
                else
                {
                    Result = wait_timeout; //time out
                }
            }
            else
            {
                pthread_cond_wait(&common_mutex_cond->i_cv,
                                  &common_mutex_cond->i_mutex);
                for (i = 0; i < Count; i++)
                {
                    if (common_mutex_cond->i_sigevent == Events[i]->GetHandle()->getev())
                    {
                        Result = i; // or return i obj
                        break;
                    }
                }

                if (i == Count)
                { // not find common_mutex_cond->i_sigevent == Events[i]
                    assert(0);
                }
            }
        }

        ret = pthread_mutex_unlock(&(common_mutex_cond->i_mutex));
        assert(ret == 0);
        /*
         * Delete all the List elements created by this thread for
         * each event.
         */

        for (i=0; i < Count; i++)
        {
            event = Events[i]->GetHandle()->getev();

            ret = pthread_mutex_lock(&event->i_list->i_mutex);
            assert(ret == 0);

            if ((i == Result) && (event->i_manual_reset != 1))
            {
                event->i_signalled = _nonsig_; // all wait thread exit reset nosingle  
            }

            deletefromlist(event, list_element[i]);
            ret = pthread_mutex_unlock(&(event->i_list->i_mutex));
            assert(ret == 0);
        }

        pthread_mutex_destroy(&(common_mutex_cond->i_mutex));
        pthread_cond_destroy(&(common_mutex_cond->i_cv));

        free(common_mutex_cond);
        common_mutex_cond = NULL;
    }

    LABLE:
    if (fWaitall == TRUE)
    {
        free(mutex_cond);
    }

    free(list_element);

    return Result;
#endif

}

#endif 
