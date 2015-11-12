/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "Q2PSaturationDiffusion.h"


template<>
InputParameters validParams<Q2PSaturationDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<UserObjectName>("fluid_density", "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredCoupledVar("porepressure_variable", "The variable representing the porepressure");
  params.addRequiredParam<UserObjectName>("fluid_relperm", "A RichardsRelPerm UserObject (eg RichardsRelPermPowerGas) that defines the gas relative permeability as a function of the saturation Variable.");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addRequiredParam<Real>("tension_limit", "Tension limit of the capillary curve.  The capillary pressure is assumed to be a quadratic function of saturation: Pc = T(1-S)^2, where S is the saturation, and T is the tension_limit");
  params.addClassDescription("Diffusion part of the Flux according to Darcy-Richards flow.  The Variable of this Kernel must be the saturation.  The capillary pressure is a quadratic function of saturation: Pc = T(1-S)^2, where T is the tension limit.");
  return params;
}

Q2PSaturationDiffusion::Q2PSaturationDiffusion(const InputParameters & parameters) :
    Kernel(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _pp(coupledValue("porepressure_variable")),
    _pp_var_num(coupled("porepressure_variable")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _ten_limit(getParam<Real>("tension_limit"))
{
}


Real
Q2PSaturationDiffusion::computeQpResidual()
{
  Real coef = 2*_ten_limit*(1-_u[_qp])*_density.density(_pp[_qp])*_relperm.relperm(_u[_qp])/_viscosity;
  return coef*_grad_test[_i][_qp]*(_permeability[_qp]*_grad_u[_qp]);
}


Real
Q2PSaturationDiffusion::computeQpJacobian()
{
  Real coef = 2*_ten_limit*(1-_u[_qp])*_density.density(_pp[_qp])*_relperm.relperm(_u[_qp])/_viscosity;
  Real coefp = 2*_ten_limit*(-_relperm.relperm(_u[_qp]) + (1 - _u[_qp])*_relperm.drelperm(_u[_qp]))*_density.density(_pp[_qp])/_viscosity;
  return coefp*_phi[_j][_qp]*_grad_test[_i][_qp]*(_permeability[_qp]*_grad_u[_qp]) + coef*_grad_test[_i][_qp]*(_permeability[_qp]*_grad_phi[_j][_qp]);
}


Real
Q2PSaturationDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _pp_var_num)
    return 0.0;
  Real coefp = 2*_ten_limit*(1 - _u[_qp])*_density.ddensity(_pp[_qp])*_relperm.relperm(_u[_qp])/_viscosity;
  return coefp*_phi[_j][_qp]*(_grad_test[_i][_qp]*(_permeability[_qp]*_grad_u[_qp]));
}
