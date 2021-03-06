#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // for error catch

// Structure for canvas
typedef struct
{
  int width;
  int height;
  char **canvas;
  char pen;
} Canvas;

typedef struct command Command;

struct command
{
  char *str;
  Command *next;
};

// Structure for history (2-D array)
typedef struct
{
  size_t bufsize;
  Command *begin;
} History;

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(FILE *fp, Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(FILE *fp,unsigned int line);
void clear_command(FILE *fp);
void clear_screen(FILE *fp);

// enum for interpret_command results
typedef enum res{ EXIT, NORMAL, COMMAND, UNKNOWN, ERROR} Result;

int max(const int a, const int b);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
void draw_rectangle(Canvas *c, const int x0, const int y0, const int height, const int width);
void draw_circle(Canvas *c, const int x0, const int y0, const int r);
void load_text(History *his, FILE *fp, Canvas *c);
Result interpret_command(const char *command, History *his, Canvas *c);
void save_history(const char *filename, History *his);
Command *push_command(History *his, const char *str);


int main(int argc, char **argv)
{
  //for history recording
  const int bufsize = 1000;
  History his = (History){ .bufsize = bufsize, .begin = NULL};

  /*his.commands = (char**)malloc(his.max_history * sizeof(char*));
  char* tmp = (char*) malloc(his.max_history * his.bufsize * sizeof(char));
  for (int i = 0 ; i < his.max_history ; i++)
    his.commands[i] = tmp + (i * his.bufsize); 
  */

  int width;
  int height;
  if (argc != 3){
    fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
    return EXIT_FAILURE;
  } else{
    char *e;
    long w = strtol(argv[1],&e,10);
    if (*e != '\0'){
      fprintf(stderr, "%s: irregular character found %s\n", argv[1],e);
      return EXIT_FAILURE;
    }
    long h = strtol(argv[2],&e,10);
    if (*e != '\0'){
      fprintf(stderr, "%s: irregular character found %s\n", argv[2],e);
      return EXIT_FAILURE;
    }
    width = (int)w;
    height = (int)h;    
  }
  char pen = '*';

  FILE *fp;
  char buf[his.bufsize];
  fp = stdout;
  Canvas *c = init_canvas(width,height, pen);

  fprintf(fp,"\n"); // required especially for windows env
  while (1) {
    print_canvas(fp,c);
    printf("* > ");
    if(fgets(buf, bufsize, stdin) == NULL) break;

    const Result r = interpret_command(buf, &his,c);
    if (r == EXIT) break;   
    if (r == NORMAL) {
      push_command(&his, buf);
    }

    rewind_screen(fp,2); // command results
    clear_command(fp); // command itself
    rewind_screen(fp, height+2); // rewind the screen to command input

  }

  clear_screen(fp);
  free_canvas(c);
  fclose(fp);

  return 0;
}

Canvas *init_canvas(int width,int height, char pen)
{
  Canvas *new = (Canvas *)malloc(sizeof(Canvas));
  new->width = width;
  new->height = height;
  new->canvas = (char **)malloc(width * sizeof(char *));

  char *tmp = (char *)malloc(width*height*sizeof(char));
  memset(tmp, ' ', width*height*sizeof(char));
  for (int i = 0 ; i < width ; i++){
    new->canvas[i] = tmp + i * height;
  }
  
  new->pen = pen;
  return new;
}

void reset_canvas(Canvas *c)
{
  const int width = c->width;
  const int height = c->height;
  memset(c->canvas[0], ' ', width*height*sizeof(char));
}


void print_canvas(FILE *fp, Canvas *c)
{
  const int height = c->height;
  const int width = c->width;
  char **canvas = c->canvas;
  
  // 上の壁
  fprintf(fp,"+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");

  // 外壁と内側
  for (int y = 0 ; y < height ; y++) {
    fprintf(fp,"|");
    for (int x = 0 ; x < width; x++){
      const char c = canvas[x][y];
      fputc(c, fp);
    }
    fprintf(fp,"|\n");
  }
  
  // 下の壁
  fprintf(fp, "+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");
  fflush(fp);
}

void free_canvas(Canvas *c)
{
  free(c->canvas[0]); //  for 2-D array free
  free(c->canvas);
  free(c);
}

void rewind_screen(FILE *fp,unsigned int line)
{
  fprintf(fp,"\e[%dA",line);
}

void clear_command(FILE *fp)
{
  fprintf(fp,"\e[2K");
}

void clear_screen(FILE *fp)
{
  fprintf(fp, "\e[2J");
}


int max(const int a, const int b)
{
  return (a > b) ? a : b;
}
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1)
{
  const int width = c->width;
  const int height = c->height;
  char pen = c->pen;
  
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  c->canvas[x0][y0] = pen;
  for (int i = 1; i <= n; i++) {
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;
    if ( (x >= 0) && (x< width) && (y >= 0) && (y < height))
      c->canvas[x][y] = pen;
  }
}

void draw_rectangle(Canvas *c, const int x0, const int y0, const int height, const int width) {
  char pen = c->pen;

  for (int i = 0; i <= width; i++) {
    int y = y0 + i;
    if ((y >= 0) && (y < c->height)) {
      if ( (x0 >= 0) && (x0 < c->width)) c->canvas[x0][y] = pen;
      if ( (x0+width >= 0) && (x0+width < c->width)) c->canvas[x0+width][y] = pen;
    }
  }
  for (int i = 0; i <= height; i++) {
    int x = x0 + i;
    if ( (x >= 0) && (x < c->width)) {
      if ( (y0 >= 0) && (y0 < c->height)) c->canvas[x][y0] = pen;
      if ( (y0+height >= 0) && (y0+height < c->height)) c->canvas[x][y0+height] = pen;
    }
  }
}

void draw_circle(Canvas *c, const int x0, const int y0, const int r) {
  char pen = c->pen;
  for (int x = 0; x < c->width; x++) {
    for (int y = 0; y < c->height; y++) {
      int d_square = (x0-x)*(x0-x) + (y0-y)*(y0-y);
      if (d_square == r*r) c->canvas[x][y] = pen;
    }
  } 
}


void save_history(const char *filename, History *his)
{
  const char *default_history_file = "history.txt";
  if (filename == NULL)
    filename = default_history_file;
  
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return;
  }
  
  for (Command *p = his->begin; p != NULL; p = p->next) {
    fprintf(fp, "%s", p->str);
  }
  fclose(fp);
}

void load_text(History *his, FILE *fp, Canvas *c) {
  char tmp_buf[20];
  while (fgets( tmp_buf, sizeof(tmp_buf), fp) != NULL) {
    interpret_command(tmp_buf, his, c);
    push_command(his, tmp_buf);
  }
}

Result interpret_command(const char *command, History *his, Canvas *c)
{
  char buf[his->bufsize];
  strcpy(buf, command);
  buf[strlen(buf) - 1] = 0; // remove the newline character at the end

  const char *s = strtok(buf, " ");

  // The first token corresponds to command
  if (strcmp(s, "line") == 0) {
    int p[4] = {0}; // p[0]: x0, p[1]: y0, p[2]: x1, p[3]: x1 
    char *b[4];
    for (int i = 0 ; i < 4; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
	      clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 4 ; i++){
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
        clear_command(stdout);
        printf("Non-int value is included.\n");
        return ERROR;
      }
      p[i] = (int)v;
    }

    draw_line(c,p[0],p[1],p[2],p[3]);
    clear_command(stdout);
    printf("1 line drawn\n");
    return NORMAL;
  }

  if (strcmp(s, "rect") == 0) {
    int p[4] = {0}; //x0, y0, height, width
    char *b[4];

    for (int i = 0 ; i < 4; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
	      clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 4 ; i++){
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
        clear_command(stdout);
        printf("Non-int value is included.\n");
        return ERROR;
      }
      p[i] = (int)v;
    }

    draw_rectangle(c,p[0],p[1],p[2],p[3]);
    clear_command(stdout);
    printf("1 rectangle drawn\n");
    return NORMAL;
  }

  if (strcmp(s, "circle") == 0) {
    int p[3] = {0}; //x0, y0, r
    char *b[3];

    for (int i = 0 ; i < 3; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
	      clear_command(stdout);
        printf("the number of point is not enough.\n");
        return ERROR;
      }
    }
    for (int i = 0 ; i < 3 ; i++){
      char *e;
      long v = strtol(b[i],&e, 10);
      if (*e != '\0'){
        clear_command(stdout);
        printf("Non-int value is included.\n");
        return ERROR;
      }
      p[i] = (int)v;
    }

    draw_circle(c,p[0], p[1], p[2]);
    clear_command(stdout);
    printf("1 circle drawn\n");
    return NORMAL;
  }

  if (strcmp(s, "load") == 0) {
    char *filename = strtok(NULL, " "); // filename
    
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
      printf("Failed to open the file %s\n", filename);
      return ERROR;
    }
    load_text(his, fp, c);
    clear_command(stdout);
    printf("loaded the file %s\n", filename);
    return COMMAND;
  }

  if (strcmp(s, "save") == 0) {
    s = strtok(NULL, " ");
    save_history(s, his);
    clear_command(stdout);
    printf("saved as \"%s\"\n",(s==NULL)?"history.txt":s);
    return COMMAND;
  }

  if (strcmp(s, "undo") == 0) {
    reset_canvas(c);
    Command *p = his->begin;
    if (p == NULL){
      clear_command(stdout);
      printf("No command in history\n");
    }
    else {
      Command *q = NULL;
      while (p->next != NULL) {
        interpret_command(p->str, his, c);
        q = p;
        p = p->next;
      }
      if (q == NULL) {
        his->begin = NULL;
      } else {
        q->next = NULL;
      }
      free(p->str);
      free(p);
      clear_command(stdout);
      printf("undo!\n");
      return COMMAND;
    }
  }

  if (strcmp(s, "quit") == 0) {
    return EXIT;
  }
  clear_command(stdout);
  printf("error: unknown command.\n");
  return UNKNOWN;
}

Command *push_command(History *his, const char *str) {
  Command *c = (Command*)malloc(sizeof(Command));
  char *s = (char*)malloc(his->bufsize);

  strcpy(s, str);
  *c = (Command){ .next = NULL, .str = s};

  Command *p = his->begin;
  if (p == NULL) {
    his->begin = c;
  }
  else {
    while (p->next != NULL) {
      p = p->next;
    }
    p->next = c;
  }
  return c;
}
