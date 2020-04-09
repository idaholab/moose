//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGBPoly.h"
#include "Material.h"

registerMooseObject("PhaseFieldApp", ACGBPoly);

InputParameters
ACGBPoly::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Grain-Boundary model concentration dependent residual");
  params.addRequiredCoupledVar("c", "Other species concentration");
  params.addParam<Real>("en_ratio", 1.0, "Ratio of surface energy to GB energy");
  return params;
}

ACGBPoly::ACGBPoly(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _c(coupledValue("c")),
    _c_var(coupled("c")),
    _mu(getMaterialProperty<Real>("mu")),
    _gamma(getMaterialProperty<Real>("gamma_asymm")),
    _en_ratio(getParam<Real>("en_ratio"))
{
}

Real
ACGBPoly::computeDFDOP(PFFunctionType type)
{
  Real mult = 2.0 * _en_ratio * _mu[_qp] * _gamma[_qp];

  Real c = _c[_qp];
  if (c < 1.0e-8)
    c = 0.0;
  if (c > 1.0)
    c = 1.0;

  switch (type)
  {
    case Residual:
      return mult * _u[_qp] * c * c;

    case Jacobian:
      return mult * _phi[_j][_qp] * c * c;
  }

  mooseError("Invalid type passed in");
}

Real
ACGBPoly::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c = _c[_qp];
  if (c < 1.0e-8)
    c = 0.0;
  if (c > 1.0)
    c = 1.0;

  if (jvar == _c_var)
  {
    Real mult = 2.0 * _en_ratio * _mu[_qp] * _gamma[_qp];
    Real dDFDOP = 2.0 * mult * _u[_qp] * c * _phi[_j][_qp];

    return _L[_qp] * _test[_i][_qp] * dDFDOP;
  }

  return 0.0;
}
