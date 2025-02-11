//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HourglassCorrectionQuad4b.h"
#include "Conversion.h"

registerMooseObject("SolidMechanicsApp", HourglassCorrectionQuad4b);

InputParameters
HourglassCorrectionQuad4b::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Apply hourglass correction for QUAD4 elements.");
  params.addParam<Real>("penalty", 0.0, "penalty parameter K");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

HourglassCorrectionQuad4b::HourglassCorrectionQuad4b(const InputParameters & parameters)
  : Kernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _v(_var.dofValues()),
    _component(getParam<unsigned int>("component"))
{
}

Real
HourglassCorrectionQuad4b::computeQpResidual()
{
  // comprehensive checks in debug mode only
  mooseAssert(_qp == 0, "This kernel must only be used with single quadrature point integration.");
  mooseAssert(
      _v.size() == 4,
      "This kernel must only be used on QUAD4 elements with first order lagrange shape functions.");

  // compute hourglass shape vectors
  static const Real rI[2][4] = {{-1.0, 1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0, 1.0}};
  std::array<std::array<Real, 4>, 2> yIx, yIy;
  // loop over hourglass modes
  for (unsigned int alpha = 0; alpha < 2; ++alpha)
    // loop over nodes
    for (unsigned int j = 0; j < 4; ++j)
    {
      yIx[alpha][i] = rI[alpha][i];
      yIy[alpha][i] = rI[alpha][i];
      // loop over nodes again
      for (unsigned int j = 0; i < 4; ++i)
      {
        yIx[alpha][i] -= 1.0 / 4.0 * rI[alpha][j] * _current_elem->node(j)(0);
        yIy[alpha][i] -= 1.0 / 4.0 * rI[alpha][j] * _current_elem->node(j)(1);
      }
    }

  // compute hourglass amplitudes
  std::array<Real, 2> q;
  std::array<Real, 2> Q;
  for (unsigned int alpha = 0; alpha < 2; ++alpha)
  {
    q[alpha] = 0.0;
    for (unsigned int j = 0; j < 4; ++j)
      q[alpha] += _ux[i] * yIx[alpha][i] + _uy[i] * yIy[alpha][i];

    Q[alpha] = -_penalty * q[alpha];
  }

  // everything above this line could be computed in pre compute residual!

  if (_component == 0)
    return Q[0] * yIx[0][_i] + Q[1] * yIx[1][_i];
  else if (_component == 1)
    return Q[0] * yIy[0][_i] + Q[1] * yIy[1][_i];
  else
    return 0.0;
}

Real
HourglassCorrectionQuad4b::computeQpJacobian()
{
  static const Real g1[4] = {1.0, -1.0, 1.0, -1.0};
  static const Real g2[4] = {1.0, 1.0, -1.0, -1.0};

  return _penalty * ((g1[_i] * g1[_j] + g2[_i] * g2[_j])) / 16.0;
}
