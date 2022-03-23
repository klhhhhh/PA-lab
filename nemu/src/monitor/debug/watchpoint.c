#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static int used_next;
static WP *wptemp;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].oldValue=0;
    wp_pool[i].hitNum=0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
  used_next=0;
}

/* TODO: Implement the functionality of watchpoint */
bool new_wp(char *args){
          if(free_==NULL)
	assert(0); 
          WP*result=free_;
          free_=free_->next;
          result->NO=used_next;
          used_next++;
          result->next=NULL;
          strcpy(result->e,args);
          result->hitNum=0;
          bool success;
          result->oldValue=expr(result->e, &success);//
          if(success==false){
                 printf("error in new_wp:expression fault\n");
                 return false;
          }
          wptemp=head;
          if(wptemp==NULL)
                 head=result;//
          else{
                while(wptemp->next!=NULL)
                       wptemp=wptemp->next;//
                wptemp->next=result;
          }
		  printf("success: set watchpoint %d ,oldValue=%d\n " , result->NO,result->oldValue);
        return true;
}

bool free_wp(int num){
	WP *thewp=NULL;//remeber the deleted point
	if(head==NULL){
		printf("no watchpoint now\n");
		return false;
	}
//judge head then in loop judge temp->next(convenient for delete)
	if(head ->NO==num){
		thewp=head;
		head=head->next;//update head list
	}
	else{
		wptemp=head;
		while(wptemp!=NULL && wptemp->next!=NULL){
			if(wptemp->next->NO==num){//find wp
				thewp=wptemp->next;
				wptemp->next=wptemp ->next->next;//update head list
				break;
			}
			wptemp=wptemp->next;//while
		}
	}
                //add free list
	if(thewp!=NULL){
		thewp->next=free_;
		free_=thewp;//update free list
		return true;
	}
	return false;
}
