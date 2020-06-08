#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#define D_PORT "11111"
#define D_DICT "dictionary.txt"
#define EXIT_USAGE_ERROR 1
#define EXIT_GETADDRINFO_ERROR 2
#define EXIT_BIND_FAILURE 3
#define EXIT_LISTEN_FAILURE 4
#define MAXLEN 64

//Dictionary
typedef struct dictionary {
    char word[MAXLEN];
    struct dictionary * next;
} dict;

//Descriptor queue
typedef struct client_descriptor_list {
    int *buf;
    int capacity;
    int front;
    int rear;
    int size;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sdList;

//Function prototype
void sbuf_init(sdList *sp, int n);
void sbuf_insert(sdList *sp, int item);
int  sbuf_remove(sdList *sp);
ssize_t readLine(int fd, void *buffer, size_t n);
int getlistenfd(char *port);
dict * add(dict *mainDict, char *word);
int search(dict *mainDict, char *word);
void *serveClient(void * args);
void sbuf_deInit(sdList *sp);

dict *wordList;
sdList client;

int main(int argc, char **argv) {
    //Getting the path to dictionary
    char path[MAXLEN] = {};
    FILE * fp;
    if(argc>=3){
        strcpy(path, argv[2]);
        if((fp = fopen(path,"r"))==NULL){
            printf("%s%s%s\n","Cannot open dictionary ", path, ". Will use default dictionary.");
            if((fp = fopen(D_DICT, "r"))==NULL){
                printf("%s\n","Cannot open default dictionary. ");
                exit(0);
            }
        }
    }
    if(path[0]=='\0'){
       if((fp = fopen(D_DICT, "r"))==NULL){
                printf("%s\n","Cannot open default dictionary. \");
                exit(0);
        }
    }

    //Adding words to list of words
    wordList = (dict*) calloc(1,sizeof(dict));
    char word[MAXLEN];
    while(fgets(word,MAXLEN-1,fp)!=NULL){
        wordList = add(wordList,word);
    }

    fclose(fp);

    int listenfd;       /* listen socket descriptor */
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    int connectedfd;
    char client_name[MAXLEN];
    char client_port[MAXLEN];
    char port[MAXLEN];

    if (argc<2) {
        strcpy(port,D_PORT);
    } else {
        strcpy(port,argv[1]);
    }

    //Get listenfd
    listenfd=getlistenfd(port);
    sbuf_init(&client, MAXLEN);
    pthread_t workers[MAXLEN];

    int i;

    //Create threads
    for (i = 0; i < MAXLEN; i++)
        pthread_create(&(workers[i]), NULL, serveClient, NULL);

    //Main thread function
    printf("%s%s\n", "waiting to connect from port ", port);
    while (1) {
        client_addr_size = sizeof(client_addr);

        if((connectedfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_size)) == -1) {
            fprintf(stderr, "accept error\n");
            continue;
        }
        if (getnameinfo((struct sockaddr *) &client_addr, client_addr_size, client_name, MAXLEN, client_port, MAXLEN, 0) != 0) {
            fprintf(stderr, "error getting name information about client\n");
        } else {
            printf("Accepted connection from %s: %s\nWaiting for handling threads\n", client_name, client_port);
        }

        sbuf_insert(&client, connectedfd);
    }

    //Join threads
    for (i = 0; i < MAXLEN; i++)
        pthread_join(workers[i], NULL);

    free(wordList);
  return 0;
}

/* given a port number or service as string, returns a
   descriptor that we can pass to accept() */
int getlistenfd(char *port) {
  int listenfd, status;
  struct addrinfo hints, *res, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM; /* TCP */
  hints.ai_family = AF_INET;	   /* IPv4 */
  if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(status));
    exit(EXIT_GETADDRINFO_ERROR);
  }
  /* try to bind to the first available address/port in the list.
     if we fail, try the next one. */
  for(p = res;p != NULL; p = p->ai_next) {
    if ((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) {
      continue;
    }

    if (bind(listenfd, p->ai_addr, p->ai_addrlen)==0) {
      break;
    }
  }
  if (listen(listenfd,0)<0) {
    close(listenfd);
    exit(EXIT_LISTEN_FAILURE);
  }
  freeaddrinfo(res);
  return listenfd;
}


/* FROM KERRISK
   Read characters from 'fd' until a newline is encountered. If a newline
   character is not encountered in the first (n - 1) bytes, then the excess
   characters are discarded. The returned string placed in 'buf' is
   null-terminated and includes the newline character if it was read in the
   first (n - 1) bytes. The function return value is the number of bytes
   placed in buffer (which includes the newline character if encountered,
   but excludes the terminating null byte). */
ssize_t readLine(int fd, void *buffer, size_t n) {
  ssize_t numRead;                    /* # of bytes fetched by last read() */
  size_t totRead;                     /* Total bytes read so far */
  char *buf;
  char ch;

  if (n <= 0 || buffer == NULL) {
    errno = EINVAL;
    return -1;
  }

  buf = buffer;                       /* No pointer arithmetic on "void *" */

  totRead = 0;
  for (;;) {
    numRead = read(fd, &ch, 1);

    if (numRead == -1) {
      if (errno == EINTR)         /* Interrupted --> restart read() */
	continue;
      else
	return -1;              /* Some other error */

    } else if (numRead == 0) {      /* EOF */
      if (totRead == 0)           /* No bytes read; return 0 */
	return 0;
      else                        /* Some bytes read; add '\0' */
	break;

    } else {                        /* 'numRead' must be 1 if we get here */
      if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
	totRead++;
	*buf++ = ch;
      }

      if (ch == '\n')
	break;
    }
  }

  *buf = '\0';
  return totRead;
}

//To add to word list
dict * add(dict *mainDict, char *word){
    dict *temp = (dict*) calloc(1,sizeof(dict));
    strcpy(temp->word,word);
    temp->next = NULL;
    if(mainDict==NULL){
        mainDict = temp;
    } else {
        temp->next = mainDict;
    }
    return temp;
}

//To search for words in word list
int search(dict *mainDict, char *src){
    dict *current = mainDict;
    char wrd[MAXLEN];
    strcpy(wrd, current->word);
    if(strcmp(wrd, src)==0){
        return 1; // Found
    }
    while (current->next!=NULL){
        current = current->next;
        char wrd[MAXLEN];
        strcpy(wrd, current->word);
        if(strcmp(wrd, src)==0){
            return 1; // Found
        }
    }
    return 0;
}

//Code for monitors
void sbuf_init(sdList *sp, int n) {
    sp->buf = calloc(n, sizeof(int));
    sp->capacity = n;
    sp->size = 0;
    sp->front = sp->rear = 0;
    sem_init(&sp->mutex, 0, 1);
    sem_init(&sp->slots, 0, n);
    sem_init(&sp->items, 0, 0);
}
void sbuf_deInit(sdList *sp){
    free(sp->buf);
}

void sbuf_insert(sdList *sp, int item) {
    sem_wait(&sp->slots);
    sem_wait(&sp->mutex);
    sp->buf[(++sp->rear) % (sp->capacity)] = item;
    sp->size++;
    sem_post(&sp->mutex);
    sem_post(&sp->items);
}

int  sbuf_remove(sdList *sp){
    int item;
    sem_wait(&sp->items);
    sem_wait(&sp->mutex);
    item = sp->buf[(++sp->front) % (sp->capacity)];
    sp->size--;
    sem_post(&sp->mutex);
    sem_post(&sp->slots);
    return item;
}


//Worker thread function
void * serveClient(void * args){
    int cfd;
    ssize_t bytes_read;
    char line[MAXLEN];
    while (1){
        cfd = sbuf_remove(&client);
        while ((bytes_read=readLine(cfd, line, MAXLEN-1))>0) {
            printf("just read %s", line);
            char result[MAXLEN*2] = {};
            if(strlen(line)>1){
                strncpy(result, line, strlen(line)-2);
            }
            if(search(wordList,line)){
                strcat(result," OK\n");
            } else {
                strcat(result," MISPELLED\n");
            }
            write(cfd, result, sizeof(result));
        }
        printf("connection closed\n");
    }
    close(cfd);
    return NULL;
}
