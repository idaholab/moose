#ifndef TEMPERATUREAUX_H
#define TEMPERATUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class TemperatureAux;

template<>
InputParameters validParams<TemperatureAux>();

/** 
 * Nodal auxiliary variable, for computing temperature at the nodes
 * based on :
 *
 * T = (rhoE / rho - u*u/2)/cv
 *
 */
class TemperatureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  TemperatureAux(const std::string & name, InputParameters parameters);

  virtual ~TemperatureAux() {}
  
protected:
  virtual Real computeValue();

  // Coupled variables
  VariableValue & _rho;
  VariableValue & _rhoE;
  VariableValue & _vel;  
  
  // Parameters
  Real _cv;   // specific heat, J/kg*K

};

#endif 
