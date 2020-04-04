//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCTradMaterial.h"

registerMooseObject("PhaseFieldApp", PFCTradMaterial);

InputParameters
PFCTradMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Polynomial coefficients for a phase field crystal correlation function");
  MooseEnum order("FOURTH=4 EIGHTH=8");
  params.addRequiredParam<MooseEnum>(
      "order", order, "This is the order of the polynomial used for correlation function");
  return params;
}

PFCTradMaterial::PFCTradMaterial(const InputParameters & parameters)
  : Material(parameters),
    _order(getParam<MooseEnum>("order")),
    _M(declareProperty<Real>("M")),
    _a(declareProperty<Real>("a")),
    _b(declareProperty<Real>("b")),
    _C0(declareProperty<Real>("C0")),
    _C2(declareProperty<Real>("C2")),
    _C4(declareProperty<Real>("C4")),
    _C6(declareProperty<Real>("C6")),
    _C8(declareProperty<Real>("C8"))
{
}

void
PFCTradMaterial::computeQpProperties()
{
  const Real invSkm = 0.332;
  const Real u_s = 0.72;

  _M[_qp] = 1.0;
  _a[_qp] = 3.0 / (2.0 * u_s) * invSkm;
  _b[_qp] = 4.0 / (30.0 * u_s * u_s) * invSkm;

  switch (_order)
  {
    case 4:
      _C0[_qp] = -10.9153;
      _C2[_qp] = 2.6;    // Angstrom^2
      _C4[_qp] = 0.1459; // Angstrom^4, would be negative but coefficient term is negative
      break;

    case 8:
      _C0[_qp] = -49.0;
      // new parameter derived from Jaatinen's paper; using km = 2.985 A; updated 1/31/2015.
      _C2[_qp] = 20.00313; // Angstrom^2
      _C4[_qp] = 3.11883;  // Angstrom^4, would be negative but coefficient term is negative
      _C6[_qp] = 0.22554;  // Angstrom^6
      _C8[_qp] = 0.00643;  // Angstrom^8, would be negative but coefficient term is negative
      break;

    default:
      mooseError("Unknown order value.");
  }
}
