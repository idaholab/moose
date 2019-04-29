#pragma once

#include "CoupledForce.h"
#include "RZSymmetry.h"

class CoupledForceRZ;

template <>
InputParameters validParams<CoupledForceRZ>();

/**
 * Source term proportional to the coupled variable in RZ coordinates
 */
class CoupledForceRZ : public CoupledForce, public RZSymmetry
{
public:
  CoupledForceRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
};
