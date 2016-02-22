#ifndef ONEDENERGYWALLHEATFLUX_H
#define ONEDENERGYWALLHEATFLUX_H

#include "Kernel.h"

// Forward Declarations
class OneDEnergyWallHeatFlux;
class Function;

template<>
InputParameters validParams<OneDEnergyWallHeatFlux>();

class OneDEnergyWallHeatFlux : public Kernel
{
public:
  OneDEnergyWallHeatFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Heat flux function
  Function & _q_wall;
  /// Heat flux perimeter
  const VariableValue & _Phf;
};

#endif //ONEDENERGYWALLHEATFLUX_H
