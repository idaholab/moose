/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSKernel.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template<>
InputParameters validParams<NSKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("u", "");
  params.addCoupledVar("v", ""); // only required in 2D and 3D
  params.addCoupledVar("w", ""); // only required in 3D
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addCoupledVar("rhov", "y-momentum"); // only required in 2D and 3D
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoE", "total energy");
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");
  return params;
}

NSKernel::NSKernel(const InputParameters & parameters) :
    Kernel(parameters),
    // Coupled variables
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),

    _rho(coupledValue("rho")),
    _rho_u(coupledValue("rhou")),
    _rho_v( _mesh.dimension() >= 2 ? coupledValue("rhov") : _zero),
    _rho_w( _mesh.dimension() == 3 ? coupledValue("rhow") : _zero),
    _rho_E(coupledValue("rhoE")),

    // Gradients
    _grad_rho(coupledGradient("rho")),
    _grad_rho_u(coupledGradient("rhou")),
    _grad_rho_v( _mesh.dimension() >= 2 ? coupledGradient("rhov") : _grad_zero),
    _grad_rho_w( _mesh.dimension() == 3 ? coupledGradient("rhow") : _grad_zero),
    _grad_rho_E(coupledGradient("rhoE")),

    // Variable numberings
    _rho_var_number( coupled("rho") ),
    _rhou_var_number( coupled("rhou") ),
    _rhov_var_number( _mesh.dimension() >= 2 ? coupled("rhov") : libMesh::invalid_uint),
    _rhow_var_number( _mesh.dimension() == 3 ? coupled("rhow") : libMesh::invalid_uint),
    _rhoE_var_number( coupled("rhoE") ),

    // Material properties
    _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
    _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),

    // FluidProperties UserObject
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

unsigned
NSKernel::mapVarNumber(unsigned var)
{
  // Convert the Moose numbering to:
  // 0 for rho
  // 1 for rho*u
  // 2 for rho*v
  // 3 for rho*w
  // 4 for rho*e
  // regardless of the problem dimension, etc.
  unsigned mapped_var_number = 99;

  if (var == _rho_var_number)
    mapped_var_number = 0;
  else if (var == _rhou_var_number)
    mapped_var_number = 1;
  else if (var == _rhov_var_number)
    mapped_var_number = 2;
  else if (var == _rhow_var_number)
    mapped_var_number = 3;
  else if (var == _rhoE_var_number)
    mapped_var_number = 4;
  else
    mooseError("Invalid var!");

  return mapped_var_number;
}
