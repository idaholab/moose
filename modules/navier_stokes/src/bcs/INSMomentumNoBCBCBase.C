//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumNoBCBCBase.h"
#include "MooseMesh.h"
#include "NS.h"

InputParameters
INSMomentumNoBCBCBase::validParams()
{
  InputParameters params = IntegratedBC::validParams();

  params.addClassDescription("Base class for the 'No BC' boundary condition.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  // Required parameters
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");
  params.addParam<bool>("integrate_p_by_parts",
                        true,
                        "Allows simulations to be run with pressure BC if set to false");

  // Optional parameters
  params.addParam<MaterialPropertyName>("mu_name", "mu", "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");

  return params;
}

INSMomentumNoBCBCBase::INSMomentumNoBCBCBase(const InputParameters & parameters)
  : IntegratedBC(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _p(coupledValue(NS::pressure)),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),
    _p_var_number(coupled(NS::pressure)),

    // Required parameters
    _gravity(getParam<RealVectorValue>("gravity")),
    _component(getParam<unsigned>("component")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts")),

    // Material properties
    _mu(getMaterialProperty<Real>("mu_name")),
    _rho(getMaterialProperty<Real>("rho_name"))
{
}
