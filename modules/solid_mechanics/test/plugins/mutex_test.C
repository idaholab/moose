//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/***************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
***************************************************************************************/

#include "SMAAspUserSubroutines.h"
#include "SMAAspUserUtilities.h"
#include "MooseError.h"

using namespace libMesh;

Real mutex_test_global_thread_counter;

extern "C" void
uexternaldb_(int * LOP,
             int * /* LRESTART */,
             Real /* TIME */[],
             Real * /* DTIME */,
             int * /* KSTEP */,
             int * /* KINC */)
{
  switch (*LOP)
  {
    // beginning of the simulation
    case 0:
    {
      MutexInit(2);
      MutexLock(2);
      mutex_test_global_thread_counter = 0;
      MutexUnlock(2);
      break;
    }

    // beginning of the timestep
    case 1:
    {
      for (int i = 0; i < 1000; ++i)
      {
        MutexLock(2);
        mutex_test_global_thread_counter += 1;
        MutexUnlock(2);
      }
      break;
    }

    // end of the simulation
    case 3:
    {
      if (get_thread_id_() == 0)
        Moose::out << "sum = " << mutex_test_global_thread_counter << ".\n";
      break;
    }
  }
}
