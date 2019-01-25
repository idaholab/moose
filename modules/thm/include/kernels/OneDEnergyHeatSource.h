#ifndef ONEDENERGYHEATSOURCE_H
#define ONEDENERGYHEATSOURCE_H

#include "Kernel.h"

// Forward Declarations
class OneDEnergyHeatSource;
class Function;

template <>
InputParameters validParams<OneDEnergyHeatSource>();

/**
 * Volumetric heat source
 */
class OneDEnergyHeatSource : public Kernel
{
public:
  OneDEnergyHeatSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Heat source function
  Function & _q;
  /// Cross sectional area
  const VariableValue & _area;
};

#endif // ONEDENERGYHEATSOURCE_H
