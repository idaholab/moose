/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "FP.h"
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#ifdef __APPLE__
#include <xmmintrin.h>
#elif __linux__
#include <fenv.h>
#endif

/**
 * Floating point exception handler
 *
 */
void handleFPE(int /*signo*/, siginfo_t *info, void * /*context*/)
{
  std::cout << std::endl;
  std::cout << "Floating point exception signaled (";
  switch (info->si_code)
  {
    case FPE_INTDIV: std::cerr << "integer divide by zero"; break;
    case FPE_INTOVF: std::cerr << "integer overflow"; break;
    case FPE_FLTDIV: std::cerr << "floating point divide by zero"; break;
    case FPE_FLTOVF: std::cerr << "floating point overflow"; break;
    case FPE_FLTUND: std::cerr << "floating point underflow"; break;
    case FPE_FLTRES: std::cerr << "floating point inexact result"; break;
    case FPE_FLTINV: std::cerr << "invalid floating point operation"; break;
    case FPE_FLTSUB: std::cerr << "subscript out of range"; break;
  }
  std::cout << ")!" << std::endl;

  std::cout << std::endl;
  std::cout << "To track this down, compile debug version, start debugger, set breakpoint for 'handleFPE' and run" << std::endl;
  std::cout << "In gdb do:" << std::endl;
  std::cout << "  break handleFPE" << std::endl;
  std::cout << "  run" << std::endl;
  std::cout << "  bt" << std::endl;

  exit(-2);             // MAGIC NUMBER!
}

namespace Moose{

void enableFPE(bool on/* = true*/)
{
#ifdef DEBUG
  static int flags = 0;

  if (on)
  {
    struct sigaction new_action, old_action;

#ifdef __APPLE__
    flags = _MM_GET_EXCEPTION_MASK();           // store the flags
    _MM_SET_EXCEPTION_MASK(flags & ~_MM_MASK_INVALID);
#elif __linux__
    feenableexcept(FE_DIVBYZERO | FE_INVALID);
#endif

    // Set up the structure to specify the new action.
    new_action.sa_sigaction = handleFPE;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_SIGINFO;

    sigaction (SIGFPE, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGFPE, &new_action, NULL);
  }
  else
  {
#ifdef __APPLE__
    _MM_SET_EXCEPTION_MASK(flags);
#elif __linux__
    fedisableexcept(FE_DIVBYZERO | FE_INVALID);
#endif
    signal(SIGFPE, 0);
  }
#endif // DEBUG
}

}
