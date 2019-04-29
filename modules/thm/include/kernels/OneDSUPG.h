#pragma once

#include "OneDStabilizationBase.h"

// Forward Declarations
class OneDSUPG;

template <>
InputParameters validParams<OneDSUPG>();

/**
 * This class acts as a base class for 1D equation stabilization kernels.
 */
class OneDSUPG : public OneDStabilizationBase
{
public:
  OneDSUPG(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _component;
};
