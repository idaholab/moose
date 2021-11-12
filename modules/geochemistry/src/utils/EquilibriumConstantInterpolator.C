//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumConstantInterpolator.h"
#include "MooseError.h"
#include "MooseUtils.h"
#include "libmesh/utility.h"
#include <memory> // std::make_unique

EquilibriumConstantInterpolator::EquilibriumConstantInterpolator(
    const std::vector<Real> & temperature,
    const std::vector<Real> & logk,
    const std::string type,
    const Real no_value)
  : LeastSquaresFitBase(), _linear_interp(nullptr)
{
  // The number of temperature points and logk points must be equal
  if (temperature.size() != logk.size())
    mooseError("The temperature and logk data sets must be equal in length in "
               "EquilibriumConstantInterpolator");

  // A value of 500.0 in logk signifies 'no value', so should not be used in the fit
  std::vector<Real> useful_temperature, useful_logk;
  for (unsigned int i = 0; i < temperature.size(); ++i)
    if (!MooseUtils::absoluteFuzzyEqual(logk[i], no_value))
    {
      useful_temperature.push_back(temperature[i]);
      useful_logk.push_back(logk[i]);
    }

  LeastSquaresFitBase::setVariables(useful_temperature, useful_logk);

  // Set the type of fit
  if (type == "fourth-order")
    _fit_type = FitTypeEnum::FOURTHORDER;
  else if (type == "maier-kelly")
    _fit_type = FitTypeEnum::MAIERKELLY;
  else if (type == "piecewise-linear")
  {
    _fit_type = FitTypeEnum::PIECEWISELINEAR;
    try
    {
      _linear_interp = std::make_unique<LinearInterpolation>(useful_temperature, useful_logk);
    }
    catch (std::domain_error & e)
    {
      mooseError("EquilibriumConstantInterpolation: ", e.what());
    }
  }
  else
    mooseError("Type ", type, " is not supported in EquilibriumConstantInterpolator");

  // Set the number of coefficients for the fit
  if (useful_temperature.size() >= 5)
    _num_coeff = 5;
  else if (useful_temperature.size() >= 1 && _fit_type == FitTypeEnum::PIECEWISELINEAR)
    _num_coeff = 2;
  else
  {
    _num_coeff = 2;
    _fit_type = FitTypeEnum::LINEAR;
  }

  // If the type is Maier-Kelly and the first temperature point is zero, suggest that
  // the user should use a fourth-order polynomial
  if (MooseUtils::absoluteFuzzyEqual(useful_temperature[0], 0.0) && type == "maier-kelly")
    mooseError("A Maier-Kelly fit cannot be used when the temperature points include 0. Use a "
               "fourth-order fit instead");
}

void
EquilibriumConstantInterpolator::fillMatrix()
{
  const unsigned int num_rows = _x.size();
  const unsigned int num_cols = _num_coeff;
  _matrix.resize(num_rows * num_cols);

  // Type of function depends on fit type
  switch (_fit_type)
  {
    case FitTypeEnum::FOURTHORDER:
    {
      for (unsigned int row = 0; row < num_rows; ++row)
      {
        _matrix[row] = 1.0;
        _matrix[num_rows + row] = _x[row];
        _matrix[(2 * num_rows) + row] = Utility::pow<2>(_x[row]);
        _matrix[(3 * num_rows) + row] = Utility::pow<3>(_x[row]);
        _matrix[(4 * num_rows) + row] = Utility::pow<4>(_x[row]);
      }
      break;
    }

    case FitTypeEnum::MAIERKELLY:
    {
      for (unsigned int row = 0; row < num_rows; ++row)
      {
        _matrix[row] = std::log(_x[row]);
        _matrix[num_rows + row] = 1.0;
        _matrix[(2 * num_rows) + row] = _x[row];
        _matrix[(3 * num_rows) + row] = 1.0 / _x[row];
        _matrix[(4 * num_rows) + row] = 1.0 / Utility::pow<2>(_x[row]);
      }
      break;
    }

    case FitTypeEnum::PIECEWISELINEAR:
      break; // _matrix is not used in piecewiselinear

    default: // FitTypeEnum::LINEAR
    {
      for (unsigned int row = 0; row < num_rows; ++row)
      {
        _matrix[row] = 1.0;
        _matrix[num_rows + row] = _x[row];
      }
      break;
    }
  }
}

Real
EquilibriumConstantInterpolator::sample(Real T)
{
  switch (_fit_type)
  {
    case FitTypeEnum::FOURTHORDER:
      return _coeffs[0] + _coeffs[1] * T + _coeffs[2] * Utility::pow<2>(T) +
             _coeffs[3] * Utility::pow<3>(T) + _coeffs[4] * Utility::pow<4>(T);

    case FitTypeEnum::MAIERKELLY:
      return _coeffs[0] * std::log(T) + _coeffs[1] + _coeffs[2] * T + _coeffs[3] / T +
             _coeffs[4] / Utility::pow<2>(T);

    case FitTypeEnum::PIECEWISELINEAR:
      return _linear_interp->sample(T);

    default: // FitTypeEnum::LINEAR
      return _coeffs[0] + _coeffs[1] * T;
  }
}

Real
EquilibriumConstantInterpolator::sampleDerivative(Real T)
{
  switch (_fit_type)
  {
    case FitTypeEnum::FOURTHORDER:
      return _coeffs[1] + 2.0 * _coeffs[2] * T + 3.0 * _coeffs[3] * Utility::pow<2>(T) +
             4.0 * _coeffs[4] * Utility::pow<3>(T);

    case FitTypeEnum::MAIERKELLY:
      return _coeffs[0] / T + _coeffs[2] - _coeffs[3] / Utility::pow<2>(T) -
             2.0 * _coeffs[4] / Utility::pow<3>(T);

    case FitTypeEnum::PIECEWISELINEAR:
      return _linear_interp->sampleDerivative(T);

    default: // FitTypeEnum::LINEAR:
      return _coeffs[1];
  }
}

DualReal
EquilibriumConstantInterpolator::sample(DualReal T)
{
  switch (_fit_type)
  {
    case FitTypeEnum::FOURTHORDER:
      return _coeffs[0] + _coeffs[1] * T + _coeffs[2] * Utility::pow<2>(T) +
             _coeffs[3] * Utility::pow<3>(T) + _coeffs[4] * Utility::pow<4>(T);

    case FitTypeEnum::MAIERKELLY:
      return _coeffs[0] * std::log(T) + _coeffs[1] + _coeffs[2] * T + _coeffs[3] / T +
             _coeffs[4] / Utility::pow<2>(T);

    case FitTypeEnum::LINEAR:
      return _coeffs[0] + _coeffs[1] * T;

    default:
      mooseError("Dual cannot be used for specified fit type in EquilibriumConstantInterpolator");
  }
}
