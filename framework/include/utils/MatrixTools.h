//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <petsc.h>
#include <vector>

namespace MatrixTools
{
/**
 * Inverse the dense square matrix m using LAPACK routines.  If you need to invert a matrix "in
 * place", make the two arguments the same
 * @ param m The matrix to invert
 * @ param[out] m_inv The inverse of m, which must be of the same size as m.
 * @ return if zero then the inversion was successful.  Otherwise m was not square, contained
 * illegal entries or was singular
 */
void inverse(const std::vector<std::vector<Real>> & m, std::vector<std::vector<Real>> & m_inv);

/**
 * Inverts the dense "matrix" A using LAPACK routines
 * @param A upon input this is a row vector representing a square matrix of size sqrt(n)*sqrt(n).
 * Upon output it is the inverse (as a row-vector)
 * @param n size of the vector A
 * @return if zero then inversion was successful.  Otherwise A contained illegal entries or was
 * singular
 */
void inverse(std::vector<PetscScalar> & A, unsigned int n);
}
