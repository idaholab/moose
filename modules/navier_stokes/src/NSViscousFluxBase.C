#include "NSViscousFluxBase.h"
 

template<>
InputParameters validParams<NSViscousFluxBase>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "energy");

  return params;
}




NSViscousFluxBase::NSViscousFluxBase(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      // Material properties
      _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
      _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),

      // Coupled variable values
      _rho(coupledValue("rho")),
      _rho_u(coupledValue("rhou")),
      _rho_v(coupledValue("rhov")),
      _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
      _rho_e(coupledValue("rhoe")),

      // Gradients
      _grad_rho(coupledGradient("rho")),
      _grad_rho_u(coupledGradient("rhou")),
      _grad_rho_v(coupledGradient("rhov")),
      _grad_rho_w( _dim == 3 ? coupledGradient("rhow") : _grad_zero),

      // Variable numberings
      _rho_var_number( coupled("rho") ),
      _rhou_var_number( coupled("rhou") ),
      _rhov_var_number( coupled("rhov") ),
      _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
      _rhoe_var_number( coupled("rhoe") )
{
}
