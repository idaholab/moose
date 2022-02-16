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

registerMooseObject("PorousFlowApp", PorousFlowDarcyVelocityComponentLowerDimensional);

InputParameters
PorousFlowDarcyVelocityComponentLowerDimensional::validParams()
{
  InputParameters params = PorousFlowDarcyVelocityComponent::validParams();
  params.addCoupledVar("aperture", 1.0, "Aperture of the fracture");
  params.addClassDescription(
      "Darcy velocity on a lower-dimensional element embedded in a higher-dimensional mesh.  Units "
      "m^3.s^-1.m^-2, or m.s^-1.  Darcy velocity =  -(k_ij * krel /(mu * a) (nabla_j P - w_j)), "
      "where k_ij is the permeability tensor, krel is the relative permeability, mu is the fluid "
      "viscosity, P is the fluid pressure, a is the fracture aperture and w_j is the fluid weight. "
      " The difference between this AuxKernel and PorousFlowDarcyVelocity is that this one "
      "projects gravity along the element's tangent direction.  NOTE!  For a meaningful answer, "
      "your permeability tensor must NOT contain terms that rotate tangential vectors to "
      "non-tangential vectors.");
  return params;
}

PorousFlowDarcyVelocityComponentLowerDimensional::PorousFlowDarcyVelocityComponentLowerDimensional(
    const InputParameters & parameters)
  : PorousFlowDarcyVelocityComponent(parameters), _aperture(coupledValue("aperture"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
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

  // MOOSE will automatically make grad(P) lie along the element's tangent direction
  // but we need to project gravity in that direction too

  // tang_xi is the element's tangent vector in xi direction
  const std::vector<RealGradient> & tang_xi = _assembly.getFE(FEType(), elem_dim)->get_dxyzdxi();
  RealVectorValue tangential_gravity =
      (_gravity * tang_xi[_qp] / tang_xi[_qp].norm_sq()) * tang_xi[_qp];
  if (elem_dim == 2)
  {
    // tang_eta is the element's tangent vector in eta direction
    const std::vector<RealGradient> & tang_eta =
        _assembly.getFE(FEType(), elem_dim)->get_dxyzdeta();
    const RealGradient normal_to_xi =
        tang_eta[_qp] - (tang_eta[_qp] * tang_xi[_qp] / tang_xi[_qp].norm_sq()) * tang_xi[_qp];
    tangential_gravity += (_gravity * normal_to_xi / normal_to_xi.norm_sq()) * normal_to_xi;
  }

  return -(_permeability[_qp] *
           (_grad_p[_qp][_ph] - _fluid_density_qp[_qp][_ph] * tangential_gravity) *
           _relative_permeability[_qp][_ph] / _fluid_viscosity[_qp][_ph])(_component) /
         _aperture[_qp];
}
