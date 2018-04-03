/* nodequeue.c - dequeue, enqueue */

#include <conf.h>
#include <kernel.h>
#include <lock.h>
#include <stdio.h>


/*------------------------------------------------------------------------
 * nodeenqueuetail  --	insert an item at the tail of a list
 *------------------------------------------------------------------------
 */
queuenode* nodeenqueuetail(int item, queuenode *tail)
{
	queuenode	*newnode=(queuenode *) getmem(sizeof(queuenode));
	newnode->key=item;
	newnode->next=NULL;
	if(tail==NULL){
		tail=newnode;
	}
	else{
		tail->next=newnode;
		tail=newnode;
	}
	return tail;
}

/*------------------------------------------------------------------------
 * nodeenqueuehead  --	insert an item at the head of a list
 *------------------------------------------------------------------------
 */
queuenode* nodeenqueuehead(int item, queuenode *head)
{
	queuenode	*newnode=(queuenode *) getmem(sizeof(queuenode));
	newnode->key=item;
	newnode->next=NULL;
	if(head==NULL){
		head=newnode;
	}
	else{
		newnode->next=head;
		head=newnode;
	}
	return head;
}


/*------------------------------------------------------------------------
 *  noderemoveitem  --  remove an item from the list
 *------------------------------------------------------------------------
 */
queuenode* noderemoveitem(int item,queuenode *head)
{
	queuenode *garbage;
	if(head!=NULL && head->key==item){
		garbage=head;
		head=head->next;
		freemem((queuenode *)garbage,sizeof(queuenode));
		return head;
	}
	queuenode *temp=head;
	while(temp->next!=NULL && temp->next->key!=item){
		temp=temp->next;
	}
	if(temp->next!=NULL){
		garbage=temp->next;
		temp->next=garbage->next;
		freemem((queuenode *)garbage,sizeof(queuenode));
	}
	return head;
}

/*------------------------------------------------------------------------
 *  searchitem  --  find an item in the list
 *------------------------------------------------------------------------
 */
int searchitem(int item,queuenode *head)
{
	queuenode *temp=head;
	while(temp!=NULL && temp->key!=item){
		temp=temp->next;
	}
	if(temp->key==item)
		return 1;
	return -1;
}
