/* releaseall.c - releaseall */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>

void resetpinh(int lck);
/*------------------------------------------------------------------------
 * releaseall  --  release all locks passed as argument to function
 *------------------------------------------------------------------------
 */
int releaseall (int numlocks, int ldes)
{
	if(numlocks<0){
		return SYSERR;
	}
	int *lck = (&ldes);
	int i;
	int ld;
	int returnval = OK;
	for (i=0; i<numlocks ; i++){
		ld= *lck++;
		if(release(currpid,ld)==SYSERR)
			returnval=SYSERR;
	}
	return returnval;

}

SYSCALL release(int pid,int lck)
{
	STATWORD ps;
	disable(ps);
	if(searchitem(lck,proctab[pid].lockheld)==-1 || searchitem(pid,locktab[lck].plisthead)==-1)
	{
		restore(ps);
		return SYSERR;
	}
	locktab[lck].plisthead=noderemoveitem(pid,locktab[lck].plisthead);
	proctab[pid].lockheld=noderemoveitem(lck,proctab[pid].lockheld);
	updatepinh(pid);

	if(locktab[lck].plisthead==NULL)
		locktab[lck].plisttail=NULL;
	else{
		restore(ps);
		return OK;
	}
	
	locktab[lck].ltype=LFREE;
	if(locktab[lck].waitqueue==NULL){
		restore(ps);
		return OK;
	}
	int firstprocess=locktab[lck].waitqueue->key;
	struct	sentry	*sptr;
	if(proctab[firstprocess].pwaittype==WRITE){
		removesemqueue(firstprocess,(sptr=&semaph[locktab[lck].sem])->sqhead);
		sptr->semcnt++;
		locktab[lck].waitqueue=noderemoveitem(firstprocess,locktab[lck].waitqueue);
		
		if(locktab[lck].waitqueue==NULL)
			locktab[lck].wprio=MININT;
		else
			locktab[lck].wprio=proctab[locktab[lck].waitqueue->key].waitpriority;

		if((proctab[firstprocess].pinh!=0 && locktab[lck].lprio==proctab[firstprocess].pinh) || (proctab[firstprocess].pinh==0 && locktab[lck].lprio==proctab[firstprocess].pprio))
			updatelprio(lck);

		ready(firstprocess, RESCHYES);
	}
	else{
		queuenode *temp=locktab[lck].waitqueue->next;
		
		while (temp!=NULL && proctab[temp->key].waitpriority == proctab[firstprocess].waitpriority && proctab[temp->key].pwaittype == READ)
			temp=temp->next;
		
		if(temp!=NULL && proctab[locktab[lck].waitqueue->key].waitpriority == proctab[firstprocess].waitpriority && proctab[temp->key].pwaittype == WRITE){
			if((proctab[locktab[lck].waitqueue->key].waitstarttime>=proctab[temp->key].waitstarttime) || proctab[temp->key].waitstarttime - proctab[locktab[lck].waitqueue->key].waitstarttime <=1000){
				removesemqueue(temp->key,(sptr=&semaph[locktab[lck].sem])->sqhead);
				sptr->semcnt++;
				locktab[lck].waitqueue=noderemoveitem(temp->key,locktab[lck].waitqueue);

				if((proctab[temp->key].pinh!=0 && locktab[lck].lprio==proctab[temp->key].pinh) || (proctab[temp->key].pinh==0 && locktab[lck].lprio==proctab[temp->key].pprio))
					updatelprio(lck);

				ready(temp->key, RESCHYES);
			}

		}
		else{
			temp=locktab[lck].waitqueue;
			while (temp!=NULL && proctab[temp->key].pwaittype == READ)
			{
				removesemqueue(temp->key,(sptr=&semaph[locktab[lck].sem])->sqhead);
				sptr->semcnt++;
				ready(temp->key, RESCHNO);
				locktab[lck].waitqueue=noderemoveitem(temp->key,locktab[lck].waitqueue);
				
				if(locktab[lck].waitqueue==NULL)
					locktab[lck].wprio=MININT;
				else
					locktab[lck].wprio=proctab[locktab[lck].waitqueue->key].waitpriority;

				temp=locktab[lck].waitqueue;
			}
			updatelprio(lck);
			resched();
		}
	}
	restore(ps);
	return OK;

}

void removesemqueue(int pid, int head)
{
	int next,prev;
	next = q[head].qnext;
	while (next<NPROC &&  next != pid){
		next = q[next].qnext;
	}
	dequeue(next);
}

void updatelprio(int lck)
{
	queuenode *temp=locktab[lck].waitqueue;
	locktab[lck].lprio=MININT;
	while (temp!=NULL)
	{
		if((proctab[temp->key].pinh!=0 && locktab[lck].lprio<proctab[temp->key].pinh) || (proctab[temp->key].pinh==0 && locktab[lck].lprio<proctab[temp->key].pprio))
			locktab[lck].lprio=proctab[temp->key].pprio;
		temp=temp->next;
	}
	resetpinh(lck);

}

void updatepinh(int pid)
{
	queuenode *temp=proctab[pid].lockheld;
	int maxprio=MININT;
	while (temp!=NULL)
	{
		if((locktab[temp->key].lprio>maxprio))
			maxprio=locktab[temp->key].lprio;
		temp=temp->next;
	}
	if(maxprio>proctab[pid].pprio)
		proctab[pid].pinh=maxprio;
	else
		proctab[pid].pinh=0;

}

void resetpinh(int lck){
	queuenode *temp=locktab[lck].plisthead;
	int procwaitinglock,oldpinh;
	while (temp!=NULL)
	{
		if((proctab[temp->key].pinh!=0 && locktab[lck].lprio<proctab[temp->key].pinh))
		{
			oldpinh=proctab[temp->key].pinh;
			if(locktab[lck].lprio==MININT || locktab[lck].lprio<proctab[temp->key].pprio)
				proctab[temp->key].pinh=0;
			else
				proctab[temp->key].pinh=locktab[lck].lprio;

			procwaitinglock=proctab[temp->key].lockid;
			if(procwaitinglock!=-1 && locktab[procwaitinglock].lprio==oldpinh){
				updatelprio(procwaitinglock);
			}
		}
		temp=temp->next;

	}
}

