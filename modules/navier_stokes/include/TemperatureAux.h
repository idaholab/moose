#ifndef TEMPERATUREAUX_H
#define TEMPERATUREAUX_H

#include "AuxKernel.h"

//Forward Declarations
class TemperatureAux;

template<>
InputParameters validParams<TemperatureAux>();

/** 
 * Temperature is an auxiliary value computed from the total energy
 * and the velocity magnitude (e_i = internal energy, e_t = total energy):
 * T = e_i / c_v
 *   = (e_t - |u|^2/2) / c_v
 *
 * WARNING: DO NOT USE THIS CLASS, TEMPERATURE REQUIRES SPECIFIC HEAT,
 * A MATERIAL PROPERTY.  MATERIAL PROPERTIES ARE NOT AVAILABLE FOR
 * CALCULATIONS WITHIN AUX KERNELS.  INSTEAD, WE TREAT TEMPERATURE
 * ITSELF AS A MATERIAL PROPERTY.
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

  // The temperature depends on velocities and total energy
  VariableValue & _p;
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  VariableValue & _pe;

  // No!  AuxVariables cannot use MaterialProperties in their computation, so
  // this cannot be here!
  // MaterialProperty<Real> & _c_v;

  // This is now an AuxVariable
  VariableValue & _c_v;
};

#endif //VELOCITYAUX_H
