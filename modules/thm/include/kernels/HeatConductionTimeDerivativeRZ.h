#ifndef HEATCONDUCTIONTIMEDERIVATIVERZ_H
#define HEATCONDUCTIONTIMEDERIVATIVERZ_H

#include "HeatConductionTimeDerivative.h"

class HeatConductionTimeDerivativeRZ;

template<>
InputParameters validParams<HeatConductionTimeDerivativeRZ>();

/**
 * Doing heat conduction time derivative in RZ coordinates
 *
 * NOTE: we are not using MOOSE for RZ integration, because the position of the heat structures
 * is not in the origin and the axis are aligned differently as well. The z-axis aligns with
 * x-axis and r-axis aligns with y-axis.
 */
class HeatConductionTimeDerivativeRZ : public HeatConductionTimeDerivative
{
public:
  HeatConductionTimeDerivativeRZ(const InputParameters & parameters);
  virtual ~HeatConductionTimeDerivativeRZ();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _axial_offset;
};


#endif /* HEATCONDUCTIONTIMEDERIVATIVERZ_H */
