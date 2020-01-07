//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThreadedGeneralUserObject.h"

ThreadedGeneralUserObject::ThreadedGeneralUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
#if !defined(LIBMESH_HAVE_OPENMP) && !defined(LIBMESH_HAVE_TBB_API)
  if (libMesh::n_threads() > 1)
    mooseError(name(),
               ": You cannot use threaded general user objects with pthreads. To enable this "
               "functionality configure libMesh with OpenMP or TBB.");
#endif
}

void
ThreadedGeneralUserObject::threadJoin(const UserObject &)
{
  mooseError("ThreadedGeneralUserObject failed to override threadJoin");
}
