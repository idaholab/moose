/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MatrixTools.h"

#if PETSC_VERSION_LESS_THAN(3, 5, 0)
extern "C" void FORTRAN_CALL(dgetri)(...); // matrix inversion routine from LAPACK
#endif

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
  mooseAssert(n <= std::numeric_limits<int>::max(),
              "MatrixTools::inverse - n (leading dimension) too large");

  std::vector<PetscBLASInt> ipiv(n);
  std::vector<PetscScalar> buffer(n * 64);

  // Following does a LU decomposition of "square matrix A"
  // upon return "A = P*L*U" if return_value == 0
  // Here I use quotes because A is actually an array of length n^2, not a matrix of size n-by-n
  int return_value;
  LAPACKgetrf_(reinterpret_cast<int *>(&n),
               reinterpret_cast<int *>(&n),
               &A[0],
               reinterpret_cast<int *>(&n),
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
  int buffer_size = buffer.size();
#if PETSC_VERSION_LESS_THAN(3, 5, 0)
  FORTRAN_CALL(dgetri)
  (reinterpret_cast<int *>(&n),
   &A[0],
   reinterpret_cast<int *>(&n),
   &ipiv[0],
   &buffer[0],
   &buffer_size,
   &return_value);
#else
  LAPACKgetri_(reinterpret_cast<int *>(&n),
               &A[0],
               reinterpret_cast<int *>(&n),
               &ipiv[0],
               &buffer[0],
               &buffer_size,
               &return_value);
#endif

  if (return_value != 0)
    throw MooseException(return_value < 0
                             ? "Argument " + Moose::stringify(-return_value) +
                                   " was invalid during invert in MatrixTools::inverse."
                             : "Matrix on-diagonal entry " + Moose::stringify(return_value) +
                                   " was exactly zero during invert in MatrixTools::inverse.");
}
}
