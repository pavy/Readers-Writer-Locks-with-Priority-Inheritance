/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <stdio.h>

extern unsigned long ctr1000;
queuenode* insertwaitqueue(int pid, queuenode *head);
/*------------------------------------------------------------------------
 * lock  --  acquire lock
 *------------------------------------------------------------------------
 */

int lock (int ldes1, int type, int priority)
{
//	int sem=locktab[ldes1].sem;
	struct pentry *ptr=&proctab[currpid];
	struct lentry *lptr;
	if(isbadlock(ldes1) || (lptr=&locktab[ldes1])->lstate!=ACQUIRED)
		return SYSERR;
	
	ptr->waitpriority=priority;
	if(lptr->ltype!=LFREE){
		if(lptr->ltype==WRITE || type==WRITE || lptr->wprio > priority){
			ptr->lockid=ldes1;

			if(lptr->wprio<priority)
				lptr->wprio=priority;
			
                        ptr->waitstarttime=ctr1000;
                        ptr->pwaittype=type;
                        lptr->waitqueue=insertwaitqueue(currpid,lptr->waitqueue);

			if((ptr->pinh!=0 && lptr->lprio<ptr->pinh) || (ptr->pinh==0 && lptr->lprio<ptr->pprio))
			{
				lptr->lprio=ptr->pprio;
				updatepriority(ldes1);
			}

//			ptr->waitstarttime=ctr1000;
//			ptr->pwaittype=type;
//			lptr->waitqueue=insertwaitqueue(currpid,lptr->waitqueue);
			wait(lptr->sem);
			ptr->lockid=-1;
			
			if(lptr->lstate==DELETED || searchitem(ldes1,ptr->lockheld)==-1)
				return DELETED;
		}
	}

	ptr->lockheld=nodeenqueuehead(ldes1,ptr->lockheld);
	lptr->ltype=type;
	lptr->plisttail=nodeenqueuetail(currpid,lptr->plisttail);
	if(lptr->plisthead==NULL){
		lptr->plisthead=lptr->plisttail;
	}

	return OK;
}

/*------------------------------------------------------------------------
 *  insertwaitqueue  --  insert a pid in the wait queue in descending order of priority
 *------------------------------------------------------------------------
 */
queuenode* insertwaitqueue(int pid, queuenode *head){
	queuenode* newnode=(queuenode *) getmem(sizeof(queuenode));
	newnode->key=pid;
	newnode->next=NULL;
	if(head==NULL){
		head=newnode;
	}
	else{
		if(proctab[head->key].waitpriority<proctab[pid].waitpriority){
			newnode->next=head;
			head=newnode;
		}
		else{
			queuenode *temp=head;
			while(temp->next!=NULL && proctab[temp->next->key].waitpriority>=proctab[pid].waitpriority){
				if(proctab[temp->next->key].waitpriority==proctab[pid].waitpriority && (proctab[pid].pwaittype==READ && proctab[temp->next->key].pwaittype == WRITE))
					break;
				temp=temp->next;
			}
			newnode->next=temp->next;
			temp->next=newnode;
		}
	}
	return head;

}

void updatepriority(int lck)
{
	queuenode *temp=locktab[lck].plisthead;
	int procwaitinglock;
	while (temp!=NULL)
	{
		if((proctab[temp->key].pinh!=0 && locktab[lck].lprio>proctab[temp->key].pinh) || (proctab[temp->key].pinh==0 && locktab[lck].lprio>proctab[temp->key].pprio))
		{
			proctab[temp->key].pinh=locktab[lck].lprio;
			procwaitinglock=proctab[temp->key].lockid;
			if(procwaitinglock!=-1 && locktab[procwaitinglock].lprio<proctab[temp->key].pinh){
				locktab[procwaitinglock].lprio=proctab[temp->key].pinh;
				updatepriority(procwaitinglock);
			}
		}
		temp=temp->next;

	}

}
