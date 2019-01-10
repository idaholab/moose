//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvectiveFluxCalculatorConstantVelocity.h"

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
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0)),
    _phi(_assembly.fePhi<Real>(_u_nodal->feType())),
    _grad_phi(_assembly.feGradPhi<Real>(_u_nodal->feType()))
{
}

Real
AdvectiveFluxCalculatorConstantVelocity::getInternodalVelocity(unsigned i,
                                                               unsigned j,
                                                               unsigned qp) const
{
  return (_grad_phi[i][qp] * _velocity) * _phi[j][qp];
}

Real
AdvectiveFluxCalculatorConstantVelocity::getU(dof_id_type id) const
{
  const Node & node = _mesh.getMesh().node_ref(id);

  return _u_nodal->getNodalValue(node);
}
