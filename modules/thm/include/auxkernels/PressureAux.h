#ifndef PRESSUREAUX_H
#define PRESSUREAUX_H

#include "AuxKernel.h"

// Forward Declarations
class PressureAux;

template<>
InputParameters validParams<PressureAux>();

/** 
 * Nodal auxiliary variable, for computing pressure at the nodes
 * based on the linear approximation:
 *
 * rho = rho_0 + (drho/dp)|_0 * (p - p_0) + (drho/dT)|_0 * (T - T_0)
 *
 * The values rho_0, p_0, (drho/dp)|_0, (drho/dT)|_0 and T_0 are parameters which 
 * must be given.
 * FIXME later: this should be part of EOS
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
  VariableValue & _T;  
  
  // Parameters
  Real _rho_0;   // kg/m^3
  Real _p_0;     // Pa
  Real _drho_dp; // density/pressure = s^2 / m^2 (after cancellation)
  Real _T_0;  // reference temperature
  Real _drho_dT; 

  // Typical parameter values
  // rho_0 = 1000.; // kg/m^3
  // p_0 = 1.e5; // Pa
  // drho_dp = 1.e-7; // density/pressure = s^2 / m^2 (after cancellation)
};

#endif 
