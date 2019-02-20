#ifndef ONEDMOMENTUMMASSFLOWRATETEMPERATUREBC_H
#define ONEDMOMENTUMMASSFLOWRATETEMPERATUREBC_H

#include "OneDNodalBC.h"

class OneDMomentumMassFlowRateTemperatureBC;

template <>
InputParameters validParams<OneDMomentumMassFlowRateTemperatureBC>();

/**
 * Mass flow rate (m_dot) and temperature (T) BC
 */
class OneDMomentumMassFlowRateTemperatureBC : public OneDNodalBC
{
public:
  OneDMomentumMassFlowRateTemperatureBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Real & _m_dot;
};

#endif /* ONEDMOMENTUMMASSFLOWRATETEMPERATUREBC_H */
