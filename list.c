#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int maxlen = 1000;

typedef struct node Node;


struct node 
{
  char *str;
  Node *next;
};

typedef struct list {
  Node *begin;
} List;

List *push_front(List *ls, const char *str)
{
  Node *p = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  *p = (Node){.str = s , .next = ls->begin};
  ls->begin = p;

  return ls; 
}


List *pop_front(List *ls)
{
  assert(ls->begin != NULL);
  Node *p = ls->begin;
  ls->begin = p->next;

  free(p->str);
  free(p);
  //free(ls->begin->str);
  // free(begin);

  return ls;
}

List *push_back(List *ls, const char *str)
{
  if (ls->begin == NULL) { 
    return push_front(ls, str);
  }

  Node *p = ls->begin;
  while (p->next != NULL) {
    p = p->next;
  }

  Node *q = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  *q = (Node){.str = s, .next = NULL};
  p->next = q;

  return ls;
}

// 実習1: pop_back の実装
List *pop_back(List *ls)
{
  // write an implementation.
  assert(ls->begin != NULL);

  Node *p = ls->begin;
  Node *q = ls->begin;
  while (p->next != NULL) {
    q = p;
    p = p->next;
  }
  q->next = NULL;
  free(p->str);
  free(p);

  return ls;
}


List *remove_all(List *ls)
{
  while ((ls = pop_front(ls))->begin) 
    ; // Repeat pop_front() until the list becomes empty
  return ls;
}

int main()
{
  // Node *begin = NULL; 
  List *ls = (List *)malloc(sizeof(List));
  ls->begin = NULL;

  char buf[maxlen];
  while (fgets(buf, maxlen, stdin)) {
    // begin = push_front(begin, buf);
    ls = push_front(ls, buf);
    //begin = push_back(begin, buf); // Try this instead of push_front()
  }

  //begin = pop_front(begin);  // What will happen if you do this?
  ls = pop_front(ls);
  ls = pop_back(ls);   // What will happen if you do this?
 
  //ls = remove_all(ls); // What will happen if you do this?
  
  for (const Node *p = ls->begin; p != NULL; p = p->next) {
    printf("%s", p->str);
  }
  
  return EXIT_SUCCESS;
}