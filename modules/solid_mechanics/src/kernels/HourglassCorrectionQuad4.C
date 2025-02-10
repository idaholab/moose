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
  params.addCoupledVar("displacements", "The displacements");
  params.addParam<Real>("penalty", 0.0, "penalty parameter");
  return params;
}

HourglassCorrectionQuad4::HourglassCorrectionQuad4(const InputParameters & parameters)
  : Kernel(parameters),
    _penalty(this->template getParam<Real>("penalty")),
    _ux(getVar("displacements", 0)->dofValues()),
    _uy(getVar("displacements", 1)->dofValues())
{
  if (coupledComponents("displacements") != 2)
    paramError("displacements", "Must couple exactly two displacement variables");
}

Real
HourglassCorrectionQuad4::computeQpResidual()
{
  // comprehensive checks in debug mode only
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(
      _ux.size() == 4 && _uy.size() == 4,
      "This kernel must only be used on QUAD4 elements with first order lagrange shape functions.");

  //--- Compute the two hourglass measures for each displacement component.
  // (The factor 1/4 comes from the definition.)
  const Real H1_x = (_ux[0] - _ux[1] + _ux[2] - _ux[3]) / 4.0;
  const Real H2_x = (_ux[0] + _ux[1] - _ux[2] - _ux[3]) / 4.0;

  const Real H1_y = (_uy[0] - _uy[1] + _uy[2] - _uy[3]) / 4.0;
  const Real H2_y = (_uy[0] + _uy[1] - _uy[2] - _uy[3]) / 4.0;

  //--- Define the hourglass coefficients for the two modes.
  // For the assumed _i ordering (0,1,2,3):
  static const Real g1[4] = {1.0, -1.0, 1.0, -1.0};
  static const Real g2[4] = {1.0, 1.0, -1.0, -1.0};

  //--- Compute and return the residual contribution for the current variable.
  //
  // Since this Kernel is applied to each displacement component separately,
  // we check the variable index (_var):
  //   _var == 0: x-displacement (u)
  //   _var == 1: y-displacement (v)
  // if (_var == 0)
  // {
  //   // Residual for u:
  //   return _stab_param * (g1[_i] * H1_x + g2[_i] * H2_x) / 4.0;
  // }
  // else if (_var == 1)
  // {
  //   // Residual for v:
  //   return _stab_param * (g1[_i] * H1_y + g2[_i] * H2_y) / 4.0;
  // }
  // else
  // {
  //   // Should never happen for a displacement field.
  //   return 0.0;
  // }
  mooseInfoRepeated("(g1[_i] * H1_x + g2[_i] * H2_x) / 4.0 = ", (g1[_i] * H1_x + g2[_i] * H2_x) / 4.0);

  // mooseInfoRepeated("_ux = ", Moose::stringify(_ux), "  _uy = ", Moose::stringify(_uy));
  return 0.0;
}

#if 0
Real
HourglassStabilizationReal::computeQpJacobian()
{
  //--- For a fully consistent tangent, one must account for the coupling between different nodes.
  // Here, for simplicity, we provide only a diagonal (node i) contribution.
  // The derivative of R_i with respect to the displacement at node i for the current variable is:
  //
  //   dR_i/du_i = alpha * (g1[i]^2 + g2[i]^2) / 16
  //
  // (Note: the factor 1/16 arises because the hourglass measures have a 1/4 factor.)
  static const Real g1[4] = {1.0, -1.0, 1.0, -1.0};
  static const Real g2[4] = {1.0, 1.0, -1.0, -1.0};

  if (_var == 0 || _var == 1)
  {
    return _stab_param * ((g1[_i] * g1[_i] + g2[_i] * g2[_i])) / 16.0;
  }
  else
    return 0.0;
}
#endif
