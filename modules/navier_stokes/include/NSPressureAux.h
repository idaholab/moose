#ifndef NSPRESSURE_H
#define NSPRESSURE_H

#include "AuxKernel.h"

//Forward Declarations
class NSPressureAux;

template<>
InputParameters validParams<NSPressureAux>();

/** 
 * Velocity auxiliary value
 */
class NSPressureAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSPressureAux(const std::string & name, InputParameters parameters);

  virtual ~NSPressureAux() {}
  
protected:
  virtual Real computeValue();

  VariableValue & _p;
  VariableValue & _u_vel;
  VariableValue & _v_vel;
  VariableValue & _w_vel;
  VariableValue & _pe;

  Real _gamma;
};

#endif //VELOCITYAUX_H
