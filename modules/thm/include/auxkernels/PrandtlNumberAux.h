#ifndef PRANDTLNUMBERAUX_H
#define PRANDTLNUMBERAUX_H

#include "AuxKernel.h"

class PrandtlNumberAux;
class IAPWS95FluidProperties;

template<>
InputParameters validParams<PrandtlNumberAux>();

/**
 * Computes Prandtl number
 */
class PrandtlNumberAux : public AuxKernel
{
public:
  PrandtlNumberAux(const InputParameters & parameters);
  virtual ~PrandtlNumberAux();

protected:
  virtual Real computeValue();

  /// Specific volume
  VariableValue & _v;
  /// Specific internal energy
  VariableValue & _e;

  const IAPWS95FluidProperties & _fp;
};

#endif /* PRANDTLNUMBERAUX_H */
