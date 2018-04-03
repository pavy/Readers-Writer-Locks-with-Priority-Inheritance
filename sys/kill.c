/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	int lck=pptr->lockid;
	if(lck!=-1){
		struct	sentry	*sptr;
		removesemqueue(pid,(sptr=&semaph[locktab[lck].sem])->sqhead);
		sptr->semcnt++;
		locktab[lck].waitqueue=noderemoveitem(pid,locktab[lck].waitqueue);

		if(locktab[lck].waitqueue==NULL)
			locktab[lck].wprio=MININT;
		else
			locktab[lck].wprio=proctab[locktab[lck].waitqueue->key].waitpriority;

		if((proctab[pid].pinh!=0 && locktab[lck].lprio==proctab[pid].pinh) || (proctab[pid].pinh==0 && locktab[lck].lprio==proctab[pid].pprio))
		{
			updatelprio(lck);
		}
	}

	while(pptr->lockheld!=NULL){
		release(pid,pptr->lockheld->key);
	}

	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
