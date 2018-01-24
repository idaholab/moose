//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDarcyVelocityComponentLowerDimensional.h"
#include "MooseMesh.h"
#include "Assembly.h"

template <>
InputParameters
validParams<PorousFlowDarcyVelocityComponentLowerDimensional>()
{
  InputParameters params = validParams<PorousFlowDarcyVelocityComponent>();
  params.addClassDescription(
      "Darcy velocity on a lower-dimensional element embedded in a higher-dimensional mesh.  Units "
      "m^3.s^-1.m^-2, or m.s^-1.  Darcy velocity =  -(k_ij * krel /mu (nabla_j P - w_j)), where "
      "k_ij is the permeability tensor, krel is the relative permeability, mu is the fluid "
      "viscosity, P is the fluid pressure, and w_j is the fluid weight.  The difference between "
      "this AuxKernel and PorousFlowDarcyVelocity is that this one projects gravity along the "
      "element's tangent direction.  NOTE!  For a meaningful answer, your permeability tensor must "
      "NOT contain terms that rotate tangential vectors to non-tangential vectors.");
  return params;
}

PorousFlowDarcyVelocityComponentLowerDimensional::PorousFlowDarcyVelocityComponentLowerDimensional(
    const InputParameters & parameters)
  : PorousFlowDarcyVelocityComponent(parameters),
    _tang_xi(LIBMESH_DIM + 1),
    _tang_eta(LIBMESH_DIM + 1)
{
  for (unsigned i = 0; i < LIBMESH_DIM + 1; ++i)
  {
    _tang_xi[i] = &_assembly.getFE(FEType(), i)->get_dxyzdxi();
    _tang_eta[i] = &_assembly.getFE(FEType(), i)->get_dxyzdeta();
  }
}

Real
PorousFlowDarcyVelocityComponentLowerDimensional::computeValue()
{
  const unsigned elem_dim = _current_elem->dim();
  if (elem_dim == _mesh.dimension())
    mooseError("The variable ",
               _var.name(),
               " must must be defined on lower-dimensional elements "
               "only since it employs "
               "PorousFlowDarcyVelocityComponentLowerDimensional\n");

  RealVectorValue tangential_gravity =
      (_gravity * (*_tang_xi[elem_dim])[_qp] / (*_tang_xi[elem_dim])[_qp].norm_sq()) *
      (*_tang_xi[elem_dim])[_qp];
  if (elem_dim == 2)
    tangential_gravity +=
        (_gravity * (*_tang_eta[elem_dim])[_qp] / (*_tang_eta[elem_dim])[_qp].norm_sq()) *
        (*_tang_eta[elem_dim])[_qp];
  return -(_permeability[_qp] *
           (_grad_p[_qp][_ph] - _fluid_density_qp[_qp][_ph] * tangential_gravity) *
           _relative_permeability[_qp][_ph] / _fluid_viscosity[_qp][_ph])(_component);
}
