#ifndef HEATCONDUCTIONTIMEDERIVATIVERZ_H
#define HEATCONDUCTIONTIMEDERIVATIVERZ_H

#include "HeatConductionTimeDerivative.h"
#include "RZSymmetry.h"

class HeatConductionTimeDerivativeRZ;

template <>
InputParameters validParams<HeatConductionTimeDerivativeRZ>();

/**
 * Time derivative kernel used by heat conduction equation in arbitrary RZ symmetry
 */
class HeatConductionTimeDerivativeRZ : public HeatConductionTimeDerivative, public RZSymmetry
{
public:
  HeatConductionTimeDerivativeRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // HEATCONDUCTIONTIMEDERIVATIVERZ_H
