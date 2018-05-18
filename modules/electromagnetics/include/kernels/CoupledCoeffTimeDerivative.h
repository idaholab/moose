#ifndef COUPLEDCOEFFTIMEDERIVATIVE_H
#define COUPLEDCOEFFTIMEDERIVATIVE_H

#include "CoupledTimeDerivative.h"

class CoupledCoeffTimeDerivative;

template <>
InputParameters validParams<CoupledCoeffTimeDerivative>();

/**
 *
 */
class CoupledCoeffTimeDerivative : public CoupledTimeDerivative
{
public:
  CoupledCoeffTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  Real _coeff;
};

#endif // COUPLEDCOEFFTIMEDERIVATIVE_H
