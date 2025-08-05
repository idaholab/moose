//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUArray.h"

namespace Moose
{
namespace Kokkos
{

void
free(void * ptr)
{
  ::Kokkos::kokkos_free<ExecSpace::memory_space>(ptr);
}

} // namespace Kokkos
} // namespace Moose
