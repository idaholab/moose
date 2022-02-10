//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VolumeJunctionBaseUserObject.h"
#include "ShaftConnectableUserObjectInterface.h"

/**
 * Test component for showing how to connect a junction-derived object to a shaft
 */
class ShaftConnectedTestComponentUserObject : public VolumeJunctionBaseUserObject,
                                              public ShaftConnectableUserObjectInterface
{
public:
  ShaftConnectedTestComponentUserObject(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

  virtual void getScalarEquationJacobianData(const unsigned int & equation_index,
                                             DenseMatrix<Real> & jacobian_block,
                                             std::vector<dof_id_type> & dofs_i,
                                             std::vector<dof_id_type> & dofs_j) const override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;
  const VariableValue & _jct_var;
  const VariableValue & _omega;

  /// Jacobian entries of junction variables wrt shaft variables
  std::vector<DenseMatrix<Real>> _residual_jacobian_omega_var;

public:
  static InputParameters validParams();
};
