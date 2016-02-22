#ifndef PRESSUREAUX_H
#define PRESSUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class PressureAux;
class Function;
class SinglePhaseCommonFluidProperties;

template<>
InputParameters validParams<PressureAux>();

/**
 * Computes pressure from fluid properties
 */
class PressureAux : public AuxKernel
{
public:
  PressureAux(const InputParameters & parameters);
  virtual ~PressureAux();

protected:
  virtual Real computeValue();

  const VariableValue & _rho;
  const VariableValue & _rhou;
  const VariableValue & _rhoE;

  const SinglePhaseCommonFluidProperties & _spfp;
};

#endif
