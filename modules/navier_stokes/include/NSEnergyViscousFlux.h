#ifndef NSENERGYVISCOUSFLUX_H
#define NSENERGYVISCOUSFLUX_H

#include "NSViscousFluxBase.h"


// Forward Declarations
class NSEnergyViscousFlux;

template<>
InputParameters validParams<NSEnergyViscousFlux>();


/**
 * Derived instance of the NSViscousFluxBase class
 * for the energy equations.
 */
class NSEnergyViscousFlux : public NSViscousFluxBase
{
public:

  NSEnergyViscousFlux(const std::string & name, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variable values
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  
};
 
#endif //  NSENERGYVISCOUSFLUX_H
