/* lock table declarations and defined constants			*/

#ifndef _LOCK_H_
#define _LOCK_H_


#ifndef	NLOCKS				/* set the number of processes	*/
#define	NLOCKS		50		/*  allowed if not already done	*/
#endif

#define LFREE		0
#define ACQUIRED	1

#ifndef DELETED
#define	DELETED		-1		
#endif

#ifndef READ
#define	READ		1		
#endif

#ifndef WRITE
#define	WRITE		2		
#endif

typedef struct queuenode {
	int key;
	struct queuenode *next;
}queuenode;

struct lentry
{
	int	lprio;			/* max priority	of all processes in waitqueue considering pprio	*/
	int wprio;			/* max wait priority among all process in wait queue */
	int ltype;			/* READ/WRITE */
	int lstate;			/* LFREE/DELETED/ACQUIRED */
	int sem;			/* Semaphore id */
	queuenode *plisthead;   /* head of list of process holding the lock */
	queuenode *plisttail;   /* tail of list of process holding the lock */

	queuenode *waitqueue;   /* head of the list of processes waiting for the lock in order of priority */
};

extern	struct	lentry locktab[];

extern queuenode* nodeenqueuetail(int item, queuenode *tail);
extern queuenode* nodeenqueuehead(int item, queuenode *head);
extern queuenode* noderemoveitem(int item,queuenode *head);
extern int searchitem(int item,queuenode *head);
extern void updatepriority(int lck);
extern void updatelprio(int lck);
extern void removesemqueue(int pid, int head);
extern SYSCALL release(int pid,int lck);

#define	isbadlock(l)	(l<0 || l>=NLOCKS)

#endif
