/*
 * Calculator server
 * CSF Assignment 6
 * John D'cruz, jdcruz1
 * Sky Li, yli302
 */

#include <stdio.h>      /* for snprintf */
#include <string.h>
#include "csapp.h"
#include "calc.h"

//line buff length
#define BUF_SIZE 1024 

//helper function that prints out error message (*msg) to stderr when called
void fatal(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

//info struct that contains information for worker function
struct ConnInfo {
  struct Calc* cal;
  int in;
  int out;
};



/*
 * Read lines of input, evaluate them as calculator expressions,
 * and (if evaluation was successful) print the result of each
 * expression.  Quit when "quit" command is received.
 *
 * Parameters:
 *   calc - calculator instance
 *   infd - input file descriptor
 *   outfd - output file descriptor
 *
 * Returns:
 *   0 to stop main loop, 1 if error
 */
int chat_with_client(struct Calc *calc, int infd, int outfd) {
  rio_t in;
  char linebuf[BUF_SIZE];

  /* wrap standard input (which is file descriptor 0) */
  rio_readinitb(&in, infd);

  while (1) { //so it is never-ending loop till a quit or shutdown or weird error shows up
    ssize_t n = rio_readlineb(&in, linebuf, BUF_SIZE);
    if (n <= 0) {
      /* error or end of input */
      return 1;
    } else if (strcmp(linebuf, "quit\n") == 0 || strcmp(linebuf, "quit\r\n") == 0) {
      /* quit command */
      return 1;
    } else if (strcmp(linebuf, "shutdown\n") == 0 || strcmp(linebuf, "shutdown\r\n") == 0) {
      //shutdown. return 0 so stops loop in main
      return 0;
    } else {
      /* process input line */
      int result;
      if (calc_eval(calc, linebuf, &result) == 0) {
	/* expression couldn't be evaluated */
	rio_writen(outfd, "Error\n", 6);
      } else {
	/* output result */
	int len = snprintf(linebuf, BUF_SIZE, "%d\n", result);
	if (len < BUF_SIZE) {
	  rio_writen(outfd, linebuf, len);
	}
      }
    }
  }
  return 0; //should not reach
}

/*
 * Worker function for pthread_create to assist with multithreading.
 *
 * Parameters:
 *   arg - empty argument
 */
void *worker(void *arg) {
  struct ConnInfo *info = arg;
  /*
   * set thread as detached, so its resources are automatically
   * reclaimed when it finishes
   */
  pthread_detach(pthread_self());

  /* handle client request */
  chat_with_client(info->cal, info->in, info->out);
  close(info->in);
  free(info);

  return NULL;
}



int main(int argc, char **argv) {
  //check input
  if (argc != 2) { fatal("Usage: ./calcServer <port>"); }
  int server_fd = open_listenfd(argv[1]);

  //server_fd should be bigger than 0 if successfully opened
  if (server_fd < 0) { fatal("Couldn't open server socket\n"); }

  //variable initialization
  int keep_going = 1;
  struct Calc *calc = calc_create();
  
  while (keep_going) {
    int client_fd = Accept(server_fd, NULL, NULL);
    if (client_fd > 0) {
      //note because we are basically chatting with ourselves, use client_fd for both
      //when shutdown is present, keep_going = 0 and loop stops; otherwise keep_going = 1
      //and loop continies 
      //create the Conn info
      struct ConnInfo *info = malloc(sizeof(struct ConnInfo));
      info->cal = calc;
      info->in = client_fd;
      info->out = client_fd;

      /* start new thread to handle client connection */
      pthread_t thr_id;
      if (pthread_create(&thr_id, NULL, worker, info) != 0) {
	fatal("pthread_create failed");
      }
      
    }
  }

  calc_destroy(calc);
  
  close(server_fd); // close server socket
  return 0;  
  
}
