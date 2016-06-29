#ifndef MACHNUMBERAUX_H
#define MACHNUMBERAUX_H

#include "AuxKernel.h"

class MachNumberAux;
class SinglePhaseFluidProperties;

template<>
InputParameters validParams<MachNumberAux>();

/**
 * Computes Mach number
 */
class MachNumberAux : public AuxKernel
{
public:
  MachNumberAux(const InputParameters & parameters);
  virtual ~MachNumberAux();

protected:
  virtual Real computeValue();

  const VariableValue & _u_vel;
  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};

#endif /* MACHNUMBERAUX_H */
