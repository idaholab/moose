//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialFit.h"

#include "MooseError.h"
#include "MathUtils.h"

// C++ includes
#include <fstream>
#include <algorithm>

int PolynomialFit::_file_number = 0;

PolynomialFit::PolynomialFit(const std::vector<Real> & x,
                             const std::vector<Real> & y,
                             unsigned int order,
                             bool truncate_order)
  : LeastSquaresFitBase(x, y), _order(order), _truncate_order(truncate_order)
{
  if (_x.size() == 0)
    mooseError("PolynomialFit does not allow empty input vectors");
  if (_truncate_order)
  {
    if (_x.size() <= _order)
      _order = _x.size() - 1;
  }
  else if (_x.size() <= order)
    mooseError("PolynomialFit requires an order less than the size of the input vector");

  _num_coeff = _order + 1;
}

void
PolynomialFit::fillMatrix()
{
  unsigned int num_rows = _x.size();
  unsigned int num_cols = _order + 1;
  _matrix.resize(num_rows * num_cols);

  for (unsigned int col = 0; col < num_cols; ++col)
    for (unsigned int row = 0; row < num_rows; ++row)
      _matrix[(col * num_rows) + row] = MathUtils::pow(_x[row], col);
}

Real
PolynomialFit::sample(Real x)
{
  unsigned int size = _coeffs.size();
  Real value = 0;

  Real curr_x = 1;
  for (unsigned int i = 0; i < size; ++i)
  {
    value += _coeffs[i] * curr_x;
    curr_x *= x;
  }
  return value;
}
