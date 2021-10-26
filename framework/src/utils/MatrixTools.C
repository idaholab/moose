//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatrixTools.h"

// MOOSE includes
#include "Conversion.h"
#include "MooseError.h"
#include "MooseException.h"

// PETSc includes
#include "petscblaslapack.h"

namespace MatrixTools
{
void
inverse(const std::vector<std::vector<Real>> & m, std::vector<std::vector<Real>> & m_inv)
{
  unsigned int n = m.size();

  // check the matrix m exists and is square
  if (n == 0)
    throw MooseException("Input matrix empty during matrix inversion.");
  if (n != m_inv.size() || n != m[0].size() || n != m_inv[0].size())
    throw MooseException("Input and output matrix are not same size square matrices.");

  // build the vectorial representation
  std::vector<PetscScalar> A;
  for (const auto & rowvec : m)
    for (const auto & matrix_entry : rowvec)
      A.push_back(matrix_entry);

  inverse(A, n);

  // build the inverse
  unsigned int i = 0;
  for (auto & rowvec : m_inv)
    for (auto & inv_entry : rowvec)
      inv_entry = A[i++];
}

void
inverse(std::vector<PetscScalar> & A, unsigned int n)
{
  mooseAssert(n >= 1, "MatrixTools::inverse - n (leading dimension) needs to be positive");
  mooseAssert(n <= std::numeric_limits<unsigned int>::max(),
              "MatrixTools::inverse - n (leading dimension) too large");

  std::vector<PetscBLASInt> ipiv(n);
  std::vector<PetscScalar> buffer(n * 64);

  // Following does a LU decomposition of "square matrix A"
  // upon return "A = P*L*U" if return_value == 0
  // Here I use quotes because A is actually an array of length n^2, not a matrix of size n-by-n
  PetscBLASInt return_value;
  LAPACKgetrf_(reinterpret_cast<PetscBLASInt *>(&n),
               reinterpret_cast<PetscBLASInt *>(&n),
               &A[0],
               reinterpret_cast<PetscBLASInt *>(&n),
               &ipiv[0],
               &return_value);

  if (return_value != 0)
    throw MooseException(
        return_value < 0
            ? "Argument " + Moose::stringify(-return_value) +
                  " was invalid during LU factorization in MatrixTools::inverse."
            : "Matrix on-diagonal entry " + Moose::stringify(return_value) +
                  " was exactly zero during LU factorization in MatrixTools::inverse.");

  // get the inverse of A
  PetscBLASInt buffer_size = buffer.size();
  LAPACKgetri_(reinterpret_cast<PetscBLASInt *>(&n),
               &A[0],
               reinterpret_cast<PetscBLASInt *>(&n),
               &ipiv[0],
               &buffer[0],
               &buffer_size,
               &return_value);

  if (return_value != 0)
    throw MooseException(return_value < 0
                             ? "Argument " + Moose::stringify(-return_value) +
                                   " was invalid during invert in MatrixTools::inverse."
                             : "Matrix on-diagonal entry " + Moose::stringify(return_value) +
                                   " was exactly zero during invert in MatrixTools::inverse.");
}
}
