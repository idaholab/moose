#include "NSIntegratedBC.h"

template<>
InputParameters validParams<NSIntegratedBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  
  // Required parameters
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");

  // Coupled variables
  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D

  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "energy");
  
  return params;
}



NSIntegratedBC::NSIntegratedBC(const std::string & name, InputParameters parameters)
    : IntegratedBC(name, parameters),
      
      // Required parameters
      _specified_pressure(getParam<Real>("specified_pressure")),

      // Coupled variables
      _u_vel(coupledValue("u")),
      _v_vel(coupledValue("v")),
      _w_vel(_dim == 3 ? coupledValue("w") : _zero),

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
      _grad_rho_e(coupledGradient("rhoe")),

      // Variable numberings
      _rho_var_number( coupled("rho") ),
      _rhou_var_number( coupled("rhou") ),
      _rhov_var_number( coupled("rhov") ),
      _rhow_var_number( _dim == 3 ? coupled("rhow") : libMesh::invalid_uint),
      _rhoe_var_number( coupled("rhoe") ),
      
      // Material properties
      _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")), 
      _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor"))
{
}
