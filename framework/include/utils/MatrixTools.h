/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATRIXTOOLS_H
#define MATRIXTOOLS_H

#include "MooseTypes.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "petscblaslapack.h"
#include <vector>

namespace MatrixTools
{
/**
 * Inverse the dense square matrix m using LAPACK routines.  If you need to invert a matrix "in place", make the two arguments the same
 * @ param m The matrix to invert
 * @ param[out] m_inv The inverse of m, which must be of the same size as m.
 * @ return if zero then the inversion was successful.  Otherwise m was not square, contained illegal entries or was singular
 */
void inverse(const std::vector<std::vector<Real> > & m, std::vector<std::vector<Real> > & m_inv);

/**
 * Inverts the dense "matrix" A using LAPACK routines
 * @param A upon input this is a row vector representing a square matrix of size sqrt(n)*sqrt(n).  Upon output it is the inverse (as a row-vector)
 * @param n size of the vector A
 * @return if zero then inversion was successful.  Otherwise A contained illegal entries or was singular
 */
void inverse(std::vector<PetscScalar> & A, unsigned int n);

/**
 * Exception classe that captur the things that can go wrong with matrix inversion
 */
class LUDecompositionException : public MooseException
{
public:
  LUDecompositionException() : MooseException("LU decomposition failed during matrix inversion.") {}
};

class NoInputMatrixException : public MooseException
{
public:
  NoInputMatrixException() : MooseException("Input matrix empty during matrix inversion.") {}
};

class MatrixMismatchException : public MooseException
{
public:
  MatrixMismatchException() : MooseException("Input and output matrix are not same size square matrices.") {}
};

class MatrixInversionException : public MooseException
{
public:
  MatrixInversionException(int error) :
      MooseException(error < 0 ? "Argument " + Moose::stringify(-error) + " was invalid during matrix inversion."
                               : "Matrix on-diagonal entry " + Moose::stringify(error) + " was exactly zero. Inversion failed.") {}
};
}

#endif //MATRIXTOOLS_H
