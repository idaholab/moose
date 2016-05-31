/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MatrixTools.h"

#if PETSC_VERSION_LESS_THAN(3,5,0)
  extern "C" void FORTRAN_CALL(dgetri) ( ... ); // matrix inversion routine from LAPACK
#endif

namespace MatrixTools
{
int inverse(const std::vector<std::vector<Real> > & m, std::vector<std::vector<Real> > & m_inv)
{
  unsigned int n = m.size();

  // check the matrix m exists and is square
  if (n < 1)
    return 1;
  if (n != m_inv.size())
    return 1;
  if (n != m[0].size() || n != m_inv[0].size())
    return 1;

  // build the vectorial representation
  std::vector<PetscScalar> A;
  for (const auto & rowvec : m)
    for (const auto & matrix_entry : rowvec)
      A.push_back(matrix_entry);

  int error = inverse(A, n);

  if (error != 0)
    return error;

  // build the inverse
  unsigned i = 0;
  for (auto & rowvec : m_inv)
    for (auto & inv_entry : rowvec)
      inv_entry = A[i++];

  return 0;
}

int inverse(std::vector<PetscScalar> & A, unsigned int n)
{
  int return_value;

  int ni = (int) n;
  int buffer_size = ni * 64;
  mooseAssert(ni > 0, "MatrixTools::inverse - ni is not positive");
  mooseAssert(buffer_size > 0, "MatrixTools::inverse - buffer_size is not positive");
  std::vector<PetscBLASInt> ipiv(n);
  std::vector<PetscScalar> buffer(buffer_size);

  // Following does a LU decomposition of "square matrix A"
  // upon return "A = P*L*U" if return_value == 0
  // Here i use quotes because A is actually an array of length n^2, not a matrix of size n-by-n
  LAPACKgetrf_(&ni, &ni, &A[0], &ni, &ipiv[0], &return_value);

  if (return_value != 0)
    // couldn't LU decompose because: illegal value in A; or, A singular
    return return_value;

  // get the inverse of A
#if PETSC_VERSION_LESS_THAN(3,5,0)
  FORTRAN_CALL(dgetri)(&ni, &A[0], &ni, &ipiv[0], &buffer[0], &buffer_size, &return_value);
#else
  LAPACKgetri_(&ni, &A[0], &ni, &ipiv[0], &buffer[0], &buffer_size, &return_value);
#endif

  return return_value;
}
}
