#ifndef VELOCITYAUX_H
#define VELOCITYAUX_H

#include "AuxKernel.h"

//Forward Declarations
class VelocityAux;

template<>
InputParameters validParams<VelocityAux>();

/** 
 * Velocity auxiliary value
 */
class VelocityAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VelocityAux(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~VelocityAux() {}
  
protected:
  virtual Real computeValue();

  Real & _p;
  Real & _momentum;
};

#endif //VELOCITYAUX_H
