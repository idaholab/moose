//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBWidthAnisotropy.h"

registerMooseObject("PhaseFieldApp", GBWidthAnisotropy);

InputParameters
GBWidthAnisotropy::validParams()
{
  InputParameters params = GBAnisotropyBase::validParams();
  params.addClassDescription(
      "A material to compute anisotropic grain boundary energies and mobilities with "
      "user-specified grain boundary widths, independently for each interface between grains");
  params.addRequiredParam<Real>("mu", "Prefactor of bulk free energy");
  params.addRequiredParam<Real>("kappa",
                                "Prefactor of gradient free energies for all i-j interfaces");
  return params;
}

GBWidthAnisotropy::GBWidthAnisotropy(const InputParameters & parameters)
  : GBAnisotropyBase(parameters), _mu(getParam<Real>("mu")), _kappa(getParam<Real>("kappa"))
{
  _mu_qp = _mu;
  Real g2 = 0.0;
  Real f_interf = 0.0;
  Real a = 0.0;
  Real gamma = 0.0;
  Real y = 0.0; // 1/gamma
  Real yyy = 0.0;

  for (unsigned int m = 0; m < _op_num - 1; ++m)
    for (unsigned int n = m + 1; n < _op_num; ++n)
    {
      // Convert units of mobility and energy
      _sigma[m][n] *= _JtoeV * (_length_scale * _length_scale); // eV/nm^2

      _mob[m][n] *= _time_scale / (_JtoeV * (_length_scale * _length_scale * _length_scale *
                                             _length_scale)); // Convert to nm^4/(eV*ns);
    }

  for (unsigned int m = 0; m < _op_num - 1; ++m)
    for (unsigned int n = m + 1; n < _op_num; ++n) // m<n
    {
      g2 = _sigma[m][n] * _sigma[m][n] / (_kappa * _mu_qp);
      y = -5.288 * g2 * g2 * g2 * g2 - 0.09364 * g2 * g2 * g2 + 9.965 * g2 * g2 - 8.183 * g2 +
          2.007;
      gamma = 1 / y;
      yyy = y * y * y;
      f_interf = 0.05676 * yyy * yyy - 0.2924 * yyy * y * y + 0.6367 * yyy * y - 0.7749 * yyy +
                 0.6107 * y * y - 0.4324 * y + 0.2792;
      a = std::sqrt(f_interf / g2);

      _kappa_gamma[m][n] = _kappa; // upper triangle stores the discrete set of kappa values
      _kappa_gamma[n][m] = gamma;  // lower triangle stores the discrete set of gamma values

      _a_g2[m][n] = a;  // upper triangle stores "a" data.
      _a_g2[n][m] = g2; // lower triangle stores "g2" data.
    }
}
