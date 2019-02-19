#ifndef ONEDENERGYSTATICPRESSURESUPERSONICBC_H
#define ONEDENERGYSTATICPRESSURESUPERSONICBC_H

#include "OneDEnergyFreeBC.h"

// Forward Declarations
class OneDEnergyStaticPressureSupersonicBC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<OneDEnergyStaticPressureSupersonicBC>();

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
};

#endif // ONEDENERGYSTATICPRESSURESUPERSONICBC_H
