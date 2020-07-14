#pragma once

#include "VarMaterialBase.h"

class PrimitiveVarMaterial;

declareADValidParams(PrimitiveVarMaterial);

/**
 * Material to calculate nonlinear and auxiliary variables as material
 * properties to allow seamless use of different sets of solution
 * fields in other objects. This material calculates all fields based
 * on a nonlinear variable set including pressure, velocity, and
 * fluid temperature.
 */
class PrimitiveVarMaterial : public VarMaterialBase
{
public:
  PrimitiveVarMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  virtual bool coupledAuxiliaryVariables() const override;

  virtual void setNonlinearProperties() override;

private:
  // nonlinear solution fields of pressure, temperature, and velocity
  const ADVariableValue & _var_T_fluid;
  const ADVariableValue & _var_pressure;
  const ADVariableValue & _var_vel_x;
  const ADVariableValue & _var_vel_y;
  const ADVariableValue & _var_vel_z;

  // gradients of nonlinear solution fields
  const ADVariableGradient & _var_grad_T_fluid;
  const ADVariableGradient & _var_grad_pressure;
  const ADVariableGradient & _var_grad_vel_x;
  const ADVariableGradient & _var_grad_vel_y;
  const ADVariableGradient & _var_grad_vel_z;

  // time derivatives of nonlinear solution fields
  const ADVariableValue & _var_T_fluid_dot;
  const ADVariableValue & _var_pressure_dot;
  const ADVariableValue & _var_vel_x_dot;
  const ADVariableValue & _var_vel_y_dot;
  const ADVariableValue & _var_vel_z_dot;

  // second gradients of nonlinear solution fields
  const ADVariableSecond & _var_grad_grad_T_fluid;
  const ADVariableSecond & _var_grad_grad_vel_x;
  const ADVariableSecond & _var_grad_grad_vel_y;
  const ADVariableSecond & _var_grad_grad_vel_z;

};
