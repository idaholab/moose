///* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDPENALTYINTERFACEDIFFUSION_H
#define COUPLEDPENALTYINTERFACEDIFFUSION_H

#include "InterfaceKernel.h"

// Forward Declarations
class CoupledPenaltyInterfaceDiffusion;

template <>
InputParameters validParams<CoupledPenaltyInterfaceDiffusion>();

/**
 * DG kernel for interfacing diffusion between two variables on adjacent blocks
 */
class CoupledPenaltyInterfaceDiffusion : public InterfaceKernel
{
public:
  CoupledPenaltyInterfaceDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;
  virtual void computeElementOffDiagJacobian(unsigned int jvar) override;
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar) override;

  const Real _penalty;
  const VariableValue & _master_coupled_value;
  const VariableValue & _slave_coupled_value;

  const unsigned _master_coupled_id;
  const unsigned _slave_coupled_id;
};

#endif
