#pragma once

#include "OneDIntegratedBC.h"

// Forward Declarations
class OneDMassMassFlowRateTemperatureBC;

template <>
InputParameters validParams<OneDMassMassFlowRateTemperatureBC>();

/**
 * Mass flow rate (m_dot) and temperature (T) BC
 */
class OneDMassMassFlowRateTemperatureBC : public OneDIntegratedBC
{
public:
  OneDMassMassFlowRateTemperatureBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Real & _m_dot;
};
