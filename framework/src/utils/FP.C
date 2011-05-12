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
void handleFPE(int /*sig*/)
{
  std::cout << std::endl;
  std::cout << "Floating point exception signaled!" << std::endl;
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
#ifdef __APPLE__
    flags = _MM_GET_EXCEPTION_MASK();           // store the flags
    _MM_SET_EXCEPTION_MASK(flags & ~_MM_MASK_INVALID);
    signal(SIGFPE, handleFPE);
#elif __linux__
    feenableexcept(FE_ALL_EXCEPT);
    signal(SIGFPE, handleFPE);
#endif
  }
  else
  {
#ifdef __APPLE__
    _MM_SET_EXCEPTION_MASK(flags);
    signal(SIGFPE, 0);
#elif __linux__
    fedisableexcept(FE_ALL_EXCEPT);
    signal(SIGFPE, SIG_IGN);
#endif
  }
#endif // DEBUG
}

}
