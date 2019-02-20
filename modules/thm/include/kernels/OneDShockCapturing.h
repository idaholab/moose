#ifndef ONEDSHOCKCAPTURING_H
#define ONEDSHOCKCAPTURING_H

#include "OneDStabilizationBase.h"

// Forward Declarations
class OneDShockCapturing;

template <>
InputParameters validParams<OneDShockCapturing>();

/**
 * Shock-capturing terms for the 1D mass conservation equation.
 */
class OneDShockCapturing : public OneDStabilizationBase
{
public:
  OneDShockCapturing(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif //  ONEDSHOCKCAPTURING_H
