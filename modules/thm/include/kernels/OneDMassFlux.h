#ifndef ONEDMASSFLUX_H
#define ONEDMASSFLUX_H

#include "Kernel.h"

class OneDMassFlux;

template <>
InputParameters validParams<OneDMassFlux>();

/**
 * Mass flux
 */
class OneDMassFlux : public Kernel
{
public:
  OneDMassFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const VariableValue & _rhouA;
  unsigned int _rhouA_var_number;
};

#endif /* ONEDMASSFLUX_H */
