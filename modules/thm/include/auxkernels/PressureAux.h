#ifndef PRESSUREAUX_H
#define PRESSUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class PressureAux;
class Function;
class EquationOfState;

template<>
InputParameters validParams<PressureAux>();

/** 
 * Nodal auxiliary variable for pressure.
 */
class PressureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PressureAux(const std::string & name, InputParameters parameters);

  virtual ~PressureAux() {}
  
protected:
  virtual Real computeValue();

  // Coupled variables
  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;

  const EquationOfState & _eos;
};

#endif 
