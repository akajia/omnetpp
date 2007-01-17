//=========================================================================
//  CEXCEPTION.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//   Exception classes
//
//   Author: Andras Varga
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // for exit()
#include <stdarg.h>  // for va_list
#include <signal.h>  // raise()
#include "cexception.h"
#include "csimulation.h"
#include "ccomponent.h"
#include "cmodule.h"
#include "cenvir.h"
#include "cconfig.h"


#define BUFLEN 1024
static char buffer[BUFLEN];
static char buffer2[BUFLEN];


cException::cException() : std::exception()
{
    errorcode = eCUSTOM;
    storeCtx();
    msg = "n/a";
}

cException::cException(ErrorCode errorcode...) : std::exception()
{
    va_list va;
    va_start(va, errorcode);
    init(NULL, errorcode, cErrorMessages::get(errorcode), va);
    va_end(va);
}

cException::cException(const char *msgformat...) : std::exception()
{
    va_list va;
    va_start(va, msgformat);
    init(NULL, eCUSTOM, msgformat, va);
    va_end(va);
}

cException::cException(const cObject *where, ErrorCode errorcode...) : std::exception()
{
    va_list va;
    va_start(va, errorcode);
    init(where, errorcode, cErrorMessages::get(errorcode), va);
    va_end(va);
}

cException::cException(const cObject *where, const char *msgformat...) : std::exception()
{
    va_list va;
    va_start(va, msgformat);
    init(where, eCUSTOM, msgformat, va);
    va_end(va);
}

cException::cException(const cException& e)
{
    errorcode = e.errorcode;
    msg = e.msg;
    hascontext = e.hascontext;
    contextclassname = e.contextclassname;
    contextfullpath = e.contextfullpath;
    moduleid = e.moduleid;
}

void cException::storeCtx()
{
    hascontext = simulation.context()!=NULL;
    moduleid = -1;

    if (simulation.context())
    {
        contextclassname = simulation.context()->className();
        contextfullpath = simulation.context()->fullPath().c_str();
        if (simulation.contextModule())
            moduleid = simulation.contextModule()->id();
    }
}

void cException::exitIfStartupError()
{
    if (!cStaticFlag::isSet())
    {
        ev.printfmsg("Error during startup/shutdown: %s. Aborting.", what());
        abort();
    }
}

void cException::init(const cObject *where, ErrorCode errorcode, const char *fmt, va_list va)
{
    // store error code
    errorcode = errorcode;

    // print "(%s)%s:" conditionally:
    //  - if object is the module itself: skip
    //  - if object is local in module: use fullName()
    //  - if object is somewhere else: use fullPath()
    buffer[0]='\0';
    if (where && where!=simulation.context())
    {
        // try: if context's fullpath is same as module fullpath + object fullname, no need to print path
        sprintf(buffer2,"%s.%s",(simulation.context()?simulation.context()->fullPath().c_str():""), where->fullName());
        bool needpath = strcmp(buffer2,where->fullPath().c_str())!=0;
        sprintf(buffer, "(%s)%s: ", where->className(), needpath ? where->fullPath().c_str() : where->fullName());
    }

    vsprintf(buffer+strlen(buffer),fmt,va);
    msg = buffer;

    // store context
    storeCtx();

    // if a global object's ctor/dtor throws an exception, there won't be
    // anyone around to catch it, so print it and abort here.
    exitIfStartupError();
}

//---

cTerminationException::cTerminationException(ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    init(NULL, errorcode, cErrorMessages::get(errorcode), va);
    va_end(va);
}

cTerminationException::cTerminationException(const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    init(NULL, eCUSTOM, msgformat, va);
    va_end(va);
}

//---

cRuntimeError::cRuntimeError(ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    init(NULL, errorcode, cErrorMessages::get(errorcode), va);
    va_end(va);
    breakIntoDebuggerIfRequested();
}

cRuntimeError::cRuntimeError(const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    init(NULL, eCUSTOM, msgformat, va);
    va_end(va);
    breakIntoDebuggerIfRequested();
}

cRuntimeError::cRuntimeError(const cObject *where, ErrorCode errorcode...)
{
    va_list va;
    va_start(va, errorcode);
    init(where, errorcode, cErrorMessages::get(errorcode), va);
    va_end(va);
    breakIntoDebuggerIfRequested();
}

cRuntimeError::cRuntimeError(const cObject *where, const char *msgformat...)
{
    va_list va;
    va_start(va, msgformat);
    init(where, eCUSTOM, msgformat, va);
    va_end(va);
    breakIntoDebuggerIfRequested();
}

void cRuntimeError::breakIntoDebuggerIfRequested()
{
    if (ev.debug_on_errors)
    {
        printf("\n"
               "RUNTIME ERROR. A cRuntimeError exception is about to be thrown,\n"
               "and you requested (by setting debug-on-errors=true in the ini file)\n"
               "that errors abort execution and break into the debugger.\n"
               " - on Linux or Unix-like systems: you should now probably be running the\n"
               "   simulation under gdb or another debugger. The simulation kernel will now\n"
               "   raise a SIGABRT signal which will get you into the debugger. If you're not\n"
               "   running under a debugger, you can still use the core dump for post-mortem\n"
               "   debugging.\n"
               " - on Windows: your should have a just-in-time debugger (such as\n"
               "   the Visual C++ IDE) enabled. The simulation kernel will now\n"
               "   cause a debugger interrupt to get you into the debugger -- press\n"
               "   the [Debug] button in the dialog that comes up.\n"
               "Once in the debugger, use its \"view stack trace\" command (in gdb: \"bt\")\n"
               "to see the context of the runtime error. See error text below.\n"
               "\n"
               );
        if (!hascontext)
            printf("<!> Error: %s.\n", what());
        else if (moduleID()==-1)
            printf("<!> Error in component (%s) %s: %s.\n",
                   contextClassName(), contextFullPath(), what());
        else
            printf("<!> Error in module (%s) %s (id=%d): %s.\n",
                   contextClassName(), contextFullPath(), moduleID(), what());
        fflush(stdout);

#ifdef _MSC_VER
        __asm int 3; // debugger interrupt
#else
        raise(6); // raise SIGABRT signal
#endif
    }
}


