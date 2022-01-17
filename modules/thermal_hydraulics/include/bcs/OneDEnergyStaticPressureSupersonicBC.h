#pragma once

#include "OneDEnergyFreeBC.h"

class SinglePhaseFluidProperties;

/**
 *
 */
class OneDEnergyStaticPressureSupersonicBC : public OneDEnergyFreeBC
{
public:
  OneDEnergyStaticPressureSupersonicBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();

  const bool & _reversible;
  const VariableValue & _vel_old;
  const VariableValue & _v_old;
  const VariableValue & _e_old;
  const MaterialProperty<Real> & _c;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
