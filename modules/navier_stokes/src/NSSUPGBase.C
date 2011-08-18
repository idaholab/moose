#include "NSSUPGBase.h"
 

template<>
InputParameters validParams<NSSUPGBase>()
{
  // Initialize the params object from the base class
  InputParameters params = validParams<Kernel>();

  // Required copuled variables for Jacobian terms
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "x-momentum");
  params.addRequiredCoupledVar("rhov", "y-momentum");
  params.addCoupledVar("rhow", "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar("rhoe", "energy");

  params.addRequiredCoupledVar("u", "");
  params.addRequiredCoupledVar("v", "");
  params.addCoupledVar("w", ""); // only required in 3D

  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredCoupledVar("enthalpy", "");

  // Global parameters
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("cv", "Specific heat at constant volume");

  return params;
}




NSSUPGBase::NSSUPGBase(const std::string & name, InputParameters parameters)
    : Kernel(name, parameters),
      // Material properties
      _viscous_stress_tensor(getMaterialProperty<RealTensorValue>("viscous_stress_tensor")),
      _dynamic_viscosity(getMaterialProperty<Real>("dynamic_viscosity")),
      _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
      
      // SUPG-related material properties
      _hsupg(getMaterialProperty<Real>("hsupg")),
      _tauc(getMaterialProperty<Real>("tauc")),
      _taum(getMaterialProperty<Real>("taum")),
      _taue(getMaterialProperty<Real>("taue")),
      _strong_residuals(getMaterialProperty<std::vector<Real> >("strong_residuals")),

      // _calA_0(getMaterialProperty<RealTensorValue>("calA_0")),
      // _calA_1(getMaterialProperty<RealTensorValue>("calA_1")),
      // _calA_2(getMaterialProperty<RealTensorValue>("calA_2")),
      // _calA_3(getMaterialProperty<RealTensorValue>("calA_3")),
      // _calA_4(getMaterialProperty<RealTensorValue>("calA_4")),

      // Momentum equation inviscid flux matrices
      _calA(getMaterialProperty<std::vector<RealTensorValue> >("calA")),

      // "velocity column" matrices
      _calC(getMaterialProperty<std::vector<RealTensorValue> >("calC")),

      // energy inviscid flux matrices
      _calE(getMaterialProperty<std::vector<std::vector<RealTensorValue> > >("calE")),

      // Coupled variable values
      _rho(coupledValue("rho")),
      _rho_u(coupledValue("rhou")),
      _rho_v(coupledValue("rhov")),
      _rho_w( _dim == 3 ? coupledValue("rhow") : _zero),
      _rho_e(coupledValue("rhoe")),

      // Old coupled variable values
      _rho_old(coupledValueOld("rho")),
      _rho_u_old(coupledValueOld("rhou")),
      _rho_v_old(coupledValueOld("rhov")),
      _rho_w_old( _dim == 3 ? coupledValueOld("rhow") : _zero),
      _rho_e_old(coupledValueOld("rhoe")),

      // Coupled aux variables
      _u_vel(coupledValue("u")),
      _v_vel(coupledValue("v")),
      _w_vel(_dim == 3 ? coupledValue("w") : _zero),

      _temperature(coupledValue("temperature")),
      _enthalpy(coupledValue("enthalpy")),

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

      // Global parameters
      _gamma(getParam<Real>("gamma")),
      _R(getParam<Real>("R")),
      _cv(getParam<Real>("cv"))
{
}




unsigned NSSUPGBase::map_var_number(unsigned var)
{
  // Convert the Moose numbering to:
  // 0 for rho
  // 1 for rho*u
  // 2 for rho*v
  // 3 for rho*w
  // 4 for rho*e
  // regardless of the problem dimension, etc.  Note that mapped_var_number
  // is now appropriate for indexing into SUPG matrices...
  unsigned mapped_var_number = 99;
  
  if (var == _rho_var_number)       mapped_var_number = 0;
  else if (var == _rhou_var_number) mapped_var_number = 1;
  else if (var == _rhov_var_number) mapped_var_number = 2;
  else if (var == _rhow_var_number) mapped_var_number = 3;
  else if (var == _rhoe_var_number) mapped_var_number = 4;
  else mooseError("Invalid var!");
  
  return mapped_var_number;
}
