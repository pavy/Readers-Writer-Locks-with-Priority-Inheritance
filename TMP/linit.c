#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <stdio.h>


struct lentry locktab[NLOCKS];  /* Lock table */

void linit(){
	int i;
	struct	lentry	*lptr;
	for (i=0 ; i<NLOCKS ; i++) {	/* initialize locks */
		(lptr = &locktab[i])->lstate = LFREE;
		lptr->lprio=MININT;
		lptr->wprio=MININT;
		lptr->ltype=LFREE;
		lptr->sem=-1;
		lptr->plisthead = NULL;
		lptr->plisttail = NULL;
		lptr->waitqueue = NULL;
	}
}