#ifndef HEATCONDUCTIONRZ_H
#define HEATCONDUCTIONRZ_H

#include "HeatConduction.h"
#include "RZSymmetry.h"

class HeatConductionRZ;

template <>
InputParameters validParams<HeatConductionRZ>();

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class HeatConductionRZ : public HeatConductionKernel, public RZSymmetry
{
public:
  HeatConductionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // HEATCONDUCTIONRZ_H
