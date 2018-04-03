/* ldelete.c - ldelete */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete  --  delete a lock
 *------------------------------------------------------------------------
 */
SYSCALL ldelete(int lck)
{
	STATWORD ps;
	int	pid;
	struct	lentry	*lptr;

	disable(ps);
	if (isbadlock(lck) || locktab[lck].lstate!=ACQUIRED) {
		restore(ps);
		return(SYSERR);
	}
	lptr = &locktab[lck];
	lptr->lstate = DELETED;
	lptr->ltype = LFREE;
	for(pid=0;pid<NPROC;pid++){
		if(searchitem(lck,proctab[pid].lockheld)==1)
			proctab[pid].lockheld=noderemoveitem(lck,proctab[pid].lockheld);
	}
	sdelete(lptr->sem);
	lptr->lprio=MININT;
	lptr->wprio=MININT;
	lptr->sem=-1;
	lptr->plisthead = NULL;
	lptr->plisttail = NULL;
	lptr->waitqueue = NULL;
	restore(ps);
	return(OK);
}
