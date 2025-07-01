//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_HAVE_KOKKOS

#include "GPUMatrix.h"

namespace Moose
{
namespace Kokkos
{

void
Matrix::destroy()
{
  _matrix = PETSC_NULLPTR;
  _nr = 0;

  _col_idx.destroy();
  _row_idx.destroy();
  _row_ptr.destroy();
  _val.destroy();

  _is_host = false;
  _is_alloc = false;
}

} // namespace Kokkos
} // namespace Moose

#endif
