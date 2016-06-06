#ifndef HEATCONDUCTIONRZ_H
#define HEATCONDUCTIONRZ_H

#include "HeatConduction.h"

class HeatConductionRZ;

template<>
InputParameters validParams<HeatConductionRZ>();

/**
 * Computes heat conduction in RZ coordinates
 *
 * NOTE: we are not using MOOSE for RZ integration, because the position of the heat structures
 * is not in the origin and the axis are aligned differently as well. The z-axis aligns with
 * x-axis and r-axis aligns with y-axis.
 */
class HeatConductionRZ : public HeatConductionKernel
{
public:
  HeatConductionRZ(const InputParameters & parameters);
  virtual ~HeatConductionRZ();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};


#endif /* HEATCONDUCTIONRZ_H */
