//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxCalculatorConstantVelocity.h"
#include "Assembly.h"

registerMooseObject("PorousFlowApp", AdvectiveFluxCalculatorConstantVelocity);

template <>
InputParameters
validParams<AdvectiveFluxCalculatorConstantVelocity>()
{
  InputParameters params = validParams<AdvectiveFluxCalculatorBase>();
  params.addClassDescription(
      "Compute K_ij (a measure of advective flux from node i to node j) "
      "and R+ and R- (which quantify amount of antidiffusion to add) in the "
      "Kuzmin-Turek FEM-TVD multidimensional scheme.  Constant advective velocity is assumed");
  params.addRequiredCoupledVar("u", "The variable that is being advected");
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity vector");
  return params;
}

AdvectiveFluxCalculatorConstantVelocity::AdvectiveFluxCalculatorConstantVelocity(
    const InputParameters & parameters)
  : AdvectiveFluxCalculatorBase(parameters),
    _velocity(getParam<RealVectorValue>("velocity")),
    _u_nodal(coupledDofValues("u")),
    _phi(_assembly.fePhi<Real>(getVar("u", 0)->feType())),
    _grad_phi(_assembly.feGradPhi<Real>(getVar("u", 0)->feType()))
{
}

Real
AdvectiveFluxCalculatorConstantVelocity::computeVelocity(unsigned i, unsigned j, unsigned qp) const
{
  return (_grad_phi[i][qp] * _velocity) * _phi[j][qp];
}

Real
AdvectiveFluxCalculatorConstantVelocity::computeU(unsigned i) const
{
  return _u_nodal[i];
}
