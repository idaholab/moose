#pragma once

#include "SideIntegralPostprocessor.h"

class MomentumFluxIntegral;

template <>
InputParameters validParams<MomentumFluxIntegral>();

/**
 * Computes the boundary integral of the momentum flux.
 *
 * This is used in performing conservation checks. Note that the ability to
 * satisfy the general conservation check will depend on the boundary conditions
 * used; for periodic BC, this postprocessor is not even necessary. For most
 * BC, the general conservation statement must be altered; one exception would
 * be free BC, which while not producing a well-posed problem, are useful for
 * checking conservation.
 */
class MomentumFluxIntegral : public SideIntegralPostprocessor
{
public:
  MomentumFluxIntegral(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;

  const VariableValue & _arhouA;
  const VariableValue & _velocity;
  const VariableValue & _pressure;
  const VariableValue & _area;
  const VariableValue & _alpha;
};
