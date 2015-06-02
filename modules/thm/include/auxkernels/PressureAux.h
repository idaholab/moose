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
  PressureAux(const std::string & name, InputParameters parameters);
  virtual ~PressureAux();

protected:
  virtual Real computeValue();

  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;

  const SinglePhaseCommonFluidProperties & _spfp;
};

#endif
