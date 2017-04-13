/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSMomentumNoBCBCBase.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSMomentumNoBCBCBase>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addClassDescription("Base class for the 'No BC' boundary condition.");
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  // Required parameters
  params.addRequiredParam<Real>("mu", "dynamic viscosity");
  params.addRequiredParam<Real>("rho", "density");
  params.addRequiredParam<RealVectorValue>("gravity", "Direction of the gravity vector");
  params.addRequiredParam<unsigned>(
      "component",
      "0,1,2 depending on if we are solving the x,y,z component of the momentum equation");
  params.addParam<bool>("integrate_p_by_parts",
                        true,
                        "Allows simulations to be run with pressure BC if set to false");

  return params;
}

INSMomentumNoBCBCBase::INSMomentumNoBCBCBase(const InputParameters & parameters)
  : IntegratedBC(parameters),

    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _p(coupledValue("p")),

    // Gradients
    _grad_u_vel(coupledGradient("u")),
    _grad_v_vel(_mesh.dimension() >= 2 ? coupledGradient("v") : _grad_zero),
    _grad_w_vel(_mesh.dimension() == 3 ? coupledGradient("w") : _grad_zero),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(_mesh.dimension() >= 2 ? coupled("v") : libMesh::invalid_uint),
    _w_vel_var_number(_mesh.dimension() == 3 ? coupled("w") : libMesh::invalid_uint),
    _p_var_number(coupled("p")),

    // Required parameters
    _mu(getParam<Real>("mu")),
    _rho(getParam<Real>("rho")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _component(getParam<unsigned>("component")),
    _integrate_p_by_parts(getParam<bool>("integrate_p_by_parts"))
{
}
