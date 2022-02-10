//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "ShaftConnectableUserObjectInterface.h"

/**
 * User object to provide data for a shaft-connected motor
 */
class ShaftConnectedMotorUserObject : public GeneralUserObject,
                                      public ShaftConnectableUserObjectInterface
{
public:
  ShaftConnectedMotorUserObject(const InputParameters & params);

  virtual Real getTorque() const override;
  virtual void getTorqueJacobianData(DenseMatrix<Real> & jacobian_block,
                                     std::vector<dof_id_type> & dofs_j) const override;

  virtual Real getMomentOfInertia() const override;
  virtual void getMomentOfInertiaJacobianData(DenseMatrix<Real> & jacobian_block,
                                              std::vector<dof_id_type> & dofs_j) const override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// User defined torque
  const Real & _torque;
  /// User defined moment of intertia
  const Real & _inertia;

public:
  static InputParameters validParams();
};
