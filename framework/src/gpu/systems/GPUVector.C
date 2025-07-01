//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_HAVE_KOKKOS

#include "GPUVector.h"

namespace Moose
{
namespace Kokkos
{

void
Vector::destroy()
{
  if (!_is_alloc)
    return;

  _global_vector = PETSC_NULLPTR;
  _local_vector = PETSC_NULLPTR;
  _array = PETSC_NULLPTR;
  _system = nullptr;
  _comm = nullptr;

  _send.destroy();
  _recv.destroy();
  _local.destroy();
  _ghost.destroy();

  _assemble = false;
  _is_ghosted = false;
  _is_host = false;
  _is_alloc = false;
}

}
}

#endif
