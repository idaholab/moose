///* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernelBase.h"
#include "MooseVariableFE.h"

/**
 * Interface kernel for enforcing continuity of stress and velocity
 */
class ADPenaltyVelocityContinuity : public InterfaceKernelBase
{
public:
  static InputParameters validParams();

  ADPenaltyVelocityContinuity(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeResidualAndJacobian() override;
  virtual void computeJacobian() override;
  virtual void computeElementOffDiagJacobian(unsigned int jvar) override;
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar) override;
  virtual const MooseVariableFieldBase & variable() const override;
  virtual const MooseVariableFieldBase & neighborVariable() const override;

protected:
  /// The penalty factor
  const Real _penalty;

  /// Fluid velocity variable
  const VectorMooseVariable * const _velocity_var;

  /// Fluid velocity values
  const ADVectorVariableValue & _velocity;

  /// Solid velocity values
  std::vector<const ADVariableValue *> _solid_velocities;

  /// Displacement variables
  std::vector<const MooseVariable *> _displacements;

  /// JxW with displacement derivatives
  const MooseArray<ADReal> & _ad_JxW;

  /// Coordinate transformation with displacement derivatives
  const MooseArray<ADReal> & _ad_coord;

  /// Residuals data member to avoid constant heap allocation
  std::vector<ADReal> _residuals;

  /// Jump data member to avoid constant heap allocations
  std::vector<ADRealVectorValue> _qp_jumps;
};

inline void
ADPenaltyVelocityContinuity::computeJacobian()
{
  computeResidual();
}

inline void
ADPenaltyVelocityContinuity::computeResidualAndJacobian()
{
  computeResidual();
}

inline void
ADPenaltyVelocityContinuity::computeElementOffDiagJacobian(const unsigned int jvar)
{
  if (jvar == _velocity_var->number())
    // Only need to do this once because AD does everything all at once
    computeResidual();
}

inline void
ADPenaltyVelocityContinuity::computeNeighborOffDiagJacobian(unsigned int)
{
}

inline const MooseVariableFieldBase &
ADPenaltyVelocityContinuity::variable() const
{
  return *_velocity_var;
}

inline const MooseVariableFieldBase &
ADPenaltyVelocityContinuity::neighborVariable() const
{
  if (_displacements.empty() || !_displacements.front())
    mooseError("The 'neighborVariable' method was called which requires that displacements be "
               "actual variables.");

  return *_displacements.front();
}
