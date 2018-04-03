/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	int oldprio=pptr->pprio;
	pptr->pprio = newprio;
	if(newprio>oldprio){
		if(pptr->pprio>=pptr->pinh)
			pptr->pinh=0;

		struct lentry *lptr=&locktab[pptr->lockid];
		if(lptr->lprio<pptr->pprio)
		{
			lptr->lprio=pptr->pprio;
			updatepriority(pptr->lockid);
		}
	}
	else if(newprio<oldprio && pptr->pinh==0)
	{
		updatelprio(pptr->lockid);
	}
	restore(ps);
	return(newprio);
}
