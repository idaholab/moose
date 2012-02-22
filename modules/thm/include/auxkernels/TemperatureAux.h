#ifndef TEMPERATUREAUX_H
#define TEMPERATUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class TemperatureAux;
class Function;
class EquationOfState;

template<>
InputParameters validParams<TemperatureAux>();

/** 
 * Nodal auxiliary variable for temperature.
 */
class TemperatureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  TemperatureAux(const std::string & name, InputParameters parameters);

  virtual ~TemperatureAux();
  
protected:
  virtual Real computeValue();

  // Coupled variables
  VariableValue & _rho;
  VariableValue & _rhou;
  VariableValue & _rhoE;
  
  // Reference to base Function class.  Must be cast to an EquationOfState in the
  // constructor so that we can properly call its extended interface...
  Function& _func;
  EquationOfState& _eos;
};

#endif 
