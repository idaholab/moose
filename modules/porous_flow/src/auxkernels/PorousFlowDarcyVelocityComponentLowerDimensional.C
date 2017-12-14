/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "PorousFlowDarcyVelocityComponentLowerDimensional.h"
#include "MooseMesh.h"
#include "Assembly.h"

template <>
InputParameters
validParams<PorousFlowDarcyVelocityComponentLowerDimensional>()
{
  InputParameters params = validParams<PorousFlowDarcyVelocityComponent>();
  params.addClassDescription("Darcy velocity on a lower-dimensional element embedded in a higher-dimensional mesh.  Units m^3.s^-1.m^-2, or m.s^-1.  Darcy velocity =  -(k_ij * krel /mu (nabla_j P - w_j)), where k_ij is the permeability tensor, krel is the relative permeability, mu is the fluid viscosity, P is the fluid pressure, and w_j is the fluid weight.  The difference between this AuxKernel and PorousFlowDarcyVelocity is that this one projects gravity along the element's tangent direction.  NOTE!  For a meaningful answer, your permeability tensor must NOT contain terms that rotate tangential vectors to non-tangential vectors.");
  return params;
}

PorousFlowDarcyVelocityComponentLowerDimensional::PorousFlowDarcyVelocityComponentLowerDimensional(
    const InputParameters & parameters)
  : PorousFlowDarcyVelocityComponent(parameters)
{
}

Real
PorousFlowDarcyVelocityComponentLowerDimensional::computeValue()
{
  const unsigned elem_dim = _current_elem->dim();
  if (elem_dim == _mesh.dimension())
    mooseError("The variable ", _var.name(), " must must be defined on lower-dimensional elements only since it employs PorousFlowDarcyVelocityComponentLowerDimensional\n");

  const std::vector<RealGradient> & tang1 = _subproblem.assembly(_tid).getFE(FEType(), elem_dim)->get_dxyzdxi();
  RealVectorValue tangential_gravity = (_gravity * tang1[_qp] / tang1[_qp].norm_sq()) * tang1[_qp];
  if (elem_dim == 2)
  {
    const std::vector<RealGradient> & tang2 = _subproblem.assembly(_tid).getFE(FEType(), elem_dim)->get_dxyzdeta();
    tangential_gravity += (_gravity * tang2[_qp] / tang2[_qp].norm_sq()) * tang2[_qp];
  }
  return -(_permeability[_qp] * (_grad_p[_qp][_ph] - _fluid_density_qp[_qp][_ph] * tangential_gravity) *
           _relative_permeability[_qp][_ph] / _fluid_viscosity[_qp][_ph])(_component);
}
