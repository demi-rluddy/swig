/* ----------------------------------------------------------------------------- 
 * wadtcl.c
 *
 *     Dynamically loadable Tcl module for wad.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 2000.  The University of Chicago
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

#include <tcl.h>
#include "wad.h"
#include <signal.h>

static char cvs[] = "$Header$";

/* Handler function */	
static void handler(int signo, WadFrame *frame, char *ret) {
  static char message[65536];
  static char temp[1024];
  int  len = 0;
  char *name;
  WadFrame *f;
  WadFrame *fline = 0;
  char *srcstr= 0;
  Tcl_Interp *interp;
  int err;
  char  *type;

  if (!ret) {
    wad_default_callback(signo, frame, ret);
    return;
  }

  strcpy(message,"[ C stack trace ]\n\n");
  switch(signo) {
  case SIGSEGV:
    type = (char*)"Segmentation fault.";
    break;
  case SIGBUS:
    type = (char*)"Bus error.";
    break;
  case SIGABRT:
    type = (char*)"Abort.";
    break;
  case SIGFPE:
    type = (char*)"Floating point exception.";
    break;
  default:
    type = (char*)"Unknown.";
    break;
  }

  f = frame;
  /* Find the last exception frame */
  while (!f->last) {
    f= f->next;
  }
  /* Now work backwards */
  f = f->prev;
  while (f) {
    strcat(message, f->debug_str);
    if (f->debug_srcstr) srcstr = f->debug_srcstr;
    f = f->prev;
  }
  if (srcstr) {
    strcat(message,"\n");
    strcat(message, srcstr);
    strcat(message,"\n");
  }

  if (wad_heap_overflow) {
    write(2, "WAD: Heap overflow detected.\n", 30);
    wad_default_callback(signo, frame, ret);
  }

  /* Note: if the heap is blown, there is a very good chance that this
  function will not succeed and we'll dump core.  However, the check
  above should dump a stack trace to stderr just in case we don't make it
  back. */

  /* Try to get the Tcl interpreter through magic */
  if (ret) {
    interp = (Tcl_Interp *) wad_steal_outarg(frame,ret,1,&err);
    if (err == 0) {
      Tcl_SetResult(interp,type,TCL_STATIC);
      Tcl_AddErrorInfo(interp,message);
    }
  }
}

void tclwadinit() {
  printf("WAD Enabled\n");
  wad_init();
  wad_set_callback(handler);
  wad_set_return("TclExecuteByteCode", TCL_ERROR);
  wad_set_return("EvalObjv", TCL_ERROR);
}

int Wad_Init(Tcl_Interp *interp) {
  return TCL_OK;
}
