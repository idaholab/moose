//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumConstantFit.h"
#include "MooseError.h"

EquilibriumConstantFit::EquilibriumConstantFit(const std::vector<Real> & temperature,
                                               const std::vector<Real> & logk)
  : LeastSquaresFitBase(temperature, logk)
{
  _num_coeff = 5;

  // The number of temperature points and logk points must be equal
  if (temperature.size() != logk.size())
    mooseError(
        "The temperature and logk data sets must be equal in length in EquilibriumConstantFit");

  // At least five data points must be supplied for this functional fit
  if (temperature.size() < 5)
    mooseError("At least five data points are required in EquilibriumConstantFit");
}

void
EquilibriumConstantFit::fillMatrix()
{
  unsigned int num_rows = _x.size();
  unsigned int num_cols = _num_coeff;
  _matrix.resize(num_rows * num_cols);

  for (unsigned int row = 0; row < num_rows; ++row)
  {
    _matrix[row] = std::log(_x[row]);
    _matrix[num_rows + row] = 1.0;
    _matrix[(2 * num_rows) + row] = _x[row];
    _matrix[(3 * num_rows) + row] = 1.0 / _x[row];
    _matrix[(4 * num_rows) + row] = 1.0 / _x[row] / _x[row];
  }
}

Real
EquilibriumConstantFit::sample(Real T)
{
  Real logK =
      _coeffs[0] * std::log(T) + _coeffs[1] + _coeffs[2] * T + _coeffs[3] / T + _coeffs[4] / T / T;

  return logK;
}
