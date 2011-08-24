#ifndef NSENERGYVISCOUSFLUX_H
#define NSENERGYVISCOUSFLUX_H

#include "NSViscousFluxBase.h"
#include "NSViscStressTensorDerivs.h"


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
  
  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself
  NSViscStressTensorDerivs<NSEnergyViscousFlux> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;
};
 
#endif //  NSENERGYVISCOUSFLUX_H
