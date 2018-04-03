/* lcreate.c - lcreate */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <proc.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lcreate  --  allocate a free/deleted lock and return its index
 *------------------------------------------------------------------------
 */
SYSCALL lcreate()
{
	STATWORD ps;    
	int	lck,sem;

	disable(ps);

	for (lck=0 ; lck<NLOCKS ; lck++) {
		if (locktab[lck].lstate==LFREE || locktab[lck].lstate==DELETED) {
			if((sem=screate(0))==SYSERR){
				restore(ps);
				return(SYSERR);
			}
			locktab[lck].lstate = ACQUIRED;
			locktab[lck].sem=sem;
			locktab[lck].lprio=MININT;
			locktab[lck].wprio=MININT;
			locktab[lck].plisthead = NULL;
			locktab[lck].plisttail = NULL;
			locktab[lck].waitqueue = NULL;
			restore(ps);
			return(lck);
		}
	}
	restore(ps);
	return(SYSERR);
}
