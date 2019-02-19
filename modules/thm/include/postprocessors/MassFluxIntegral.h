#ifndef MASSFLUXINTEGRAL_H
#define MASSFLUXINTEGRAL_H

#include "SideIntegralPostprocessor.h"

class MassFluxIntegral;

template <>
InputParameters validParams<MassFluxIntegral>();

/**
 * Computes the boundary integral of the mass flux.
 *
 * This is used in performing conservation checks. Note that the ability to
 * satisfy the general conservation check will depend on the boundary conditions
 * used; for periodic BC, this postprocessor is not even necessary. For most
 * BC, the general conservation statement must be altered; one exception would
 * be free BC, which while not producing a well-posed problem, are useful for
 * checking conservation.
 */
class MassFluxIntegral : public SideIntegralPostprocessor
{
public:
  MassFluxIntegral(const InputParameters & parameters);

  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() override;

  const VariableValue & _arhouA;
};

#endif // MASSFLUXINTEGRAL_H
