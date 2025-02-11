//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HourglassCorrectionQuad4.h"
#include "Conversion.h"

registerMooseObject("SolidMechanicsApp", HourglassCorrectionQuad4);

InputParameters
HourglassCorrectionQuad4::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Apply hourglass correction for QUAD4 elements.");
  params.addParam<Real>("penalty", 0.0, "penalty parameter");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HourglassCorrectionQuad4::HourglassCorrectionQuad4(const InputParameters & parameters)
  : Kernel(parameters), _penalty(getParam<Real>("penalty")), _v(_var.dofValues())
{
}

Real
HourglassCorrectionQuad4::computeQpResidual()
{
  // comprehensive checks in debug mode only
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(
      _v.size() == 4,
      "This kernel must only be used on QUAD4 elements with first order lagrange shape functions.");

  //--- Compute the two hourglass measures.
  // (The factor 1/4 comes from the definition.)
  const Real H1 = (_v[0] - _v[1] + _v[2] - _v[3]) / 4.0;
  const Real H2 = (_v[0] + _v[1] - _v[2] - _v[3]) / 4.0;

  //--- Define the hourglass coefficients for the two modes.
  // For the assumed _i ordering (0,1,2,3):
  static const Real g1[4] = {1.0, -1.0, 1.0, -1.0};
  static const Real g2[4] = {1.0, 1.0, -1.0, -1.0};

  // mooseInfoRepeated("(g1[_i] * H1 + g2[_i] * H2) / 4.0 = ", (g1[_i] * H1 + g2[_i] * H2) / 4.0);
  return _penalty * (g1[_i] * H1 + g2[_i] * H2) / 4.0;
}

Real
HourglassCorrectionQuad4::computeQpJacobian()
{
  static const Real g1[4] = {1.0, -1.0, 1.0, -1.0};
  static const Real g2[4] = {1.0, 1.0, -1.0, -1.0};

  return _penalty * ((g1[_i] * g1[_j] + g2[_i] * g2[_j])) / 16.0;
}
