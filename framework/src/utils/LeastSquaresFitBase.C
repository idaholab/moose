//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LeastSquaresFitBase.h"
#include "MooseError.h"

extern "C" void FORTRAN_CALL(dgels)(...);

LeastSquaresFitBase::LeastSquaresFitBase() {}

LeastSquaresFitBase::LeastSquaresFitBase(const std::vector<Real> & x, const std::vector<Real> & y)
  : _x(x), _y(y)
{
}

void
LeastSquaresFitBase::setVariables(const std::vector<Real> & x, const std::vector<Real> & y)
{
  _x = x;
  _y = y;
}

void
LeastSquaresFitBase::generate()
{
  if (_x.empty())
    mooseError("Empty variables in LeastSquaresFitBase. x and y must be set in the constructor or "
               "using setVariables(x, y)");

  fillMatrix();
  doLeastSquares();
}

void
LeastSquaresFitBase::doLeastSquares()
{
  char mode = 'N';
  int num_rows = _x.size();
  unsigned int num_coeff = _num_coeff;
  int num_rhs = 1;
  int buffer_size = -1;
  Real opt_buffer_size;
  Real * buffer;
  int return_value = 0;

  // Must copy _y because the call to dgels destroys the original values
  std::vector<Real> rhs = _y;

  FORTRAN_CALL(dgels)
  (&mode,
   &num_rows,
   &num_coeff,
   &num_rhs,
   &_matrix[0],
   &num_rows,
   &rhs[0],
   &num_rows,
   &opt_buffer_size,
   &buffer_size,
   &return_value);
  if (return_value)
    throw std::runtime_error("Call to Fortran routine 'dgels' returned non-zero exit code");

  buffer_size = (int)opt_buffer_size;

  buffer = new Real[buffer_size];
  FORTRAN_CALL(dgels)
  (&mode,
   &num_rows,
   &num_coeff,
   &num_rhs,
   &_matrix[0],
   &num_rows,
   &rhs[0],
   &num_rows,
   buffer,
   &buffer_size,
   &return_value);
  delete[] buffer;

  if (return_value)
    throw std::runtime_error("Call to Fortran routine 'dgels' returned non-zero exit code");

  _coeffs.resize(num_coeff);
  for (unsigned int i = 0; i < num_coeff; ++i)
    _coeffs[i] = rhs[i];
}

unsigned int
LeastSquaresFitBase::getSampleSize()
{
  return _x.size();
}

const std::vector<Real> &
LeastSquaresFitBase::getCoefficients()
{
  return _coeffs;
}
