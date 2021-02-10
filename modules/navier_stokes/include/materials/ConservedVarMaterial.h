//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Navier-Stokes includes
#include "VarMaterialBase.h"

class ConservedVarMaterial;

declareADValidParams(ConservedVarMaterial);

/**
 * Material to calculate nonlinear and auxiliary variables as material
 * properties to allow seamless use of different sets of solution
 * fields in other objects. This material calculates all fields based
 * on a nonlinear variable set including density, momentum, and total
 * fluid energy.
 */
class ConservedVarMaterial : public VarMaterialBase
{
public:
  ConservedVarMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  virtual bool coupledAuxiliaryVariables() const override;

  virtual void setNonlinearProperties() override;

private:
  // nonlinear solution fields of density, total fluid energy, and momentum
  const ADVariableValue & _var_rho;
  const ADVariableValue & _var_total_energy_density;
  const ADVariableValue & _var_rho_u;
  const ADVariableValue & _var_rho_v;
  const ADVariableValue & _var_rho_w;

  // gradients of nonlinear solution fields
  const ADVariableGradient & _var_grad_rho;
  const ADVariableGradient & _var_grad_rho_et;
  const ADVariableGradient & _var_grad_rho_u;
  const ADVariableGradient & _var_grad_rho_v;
  const ADVariableGradient & _var_grad_rho_w;

  // second gradients of nonlinear solution fields
  const ADVariableSecond & _var_grad_grad_rho;
  const ADVariableSecond & _var_grad_grad_rho_u;
  const ADVariableSecond & _var_grad_grad_rho_v;
  const ADVariableSecond & _var_grad_grad_rho_w;

  // time derivatives of nonlinear solution fields
  const ADVariableValue & _var_rho_dot;
  const ADVariableValue & _var_rho_et_dot;
  const ADVariableValue & _var_rho_u_dot;
  const ADVariableValue & _var_rho_v_dot;
  const ADVariableValue & _var_rho_w_dot;

  // optional approximation for stabilization strong residual term
  const ADVariableSecond & _var_grad_grad_T_fluid;

};
