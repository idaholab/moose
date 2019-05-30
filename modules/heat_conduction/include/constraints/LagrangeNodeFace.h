#pragma once

#include "NodeFaceConstraint.h"

class LagrangeNodeFace;

template <>
InputParameters validParams<LagrangeNodeFace>();

/**
 *
 */
class LagrangeNodeFace : public NodeFaceConstraint
{
public:
  LagrangeNodeFace(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned jvar) override;

protected:
  virtual Real computeQpSlaveValue() override;
  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  const Real & _temp_slave;
  const Real _k;
};
