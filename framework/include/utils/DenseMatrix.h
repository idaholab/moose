//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/dense_matrix.h"

#include "DualRealOps.h"

namespace libMesh
{
template <>
DenseMatrix<DualReal>::DenseMatrix(const unsigned int new_m, const unsigned int new_n);
}
