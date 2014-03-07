#ifndef NSVELOCITYAUX_H
#define NSVELOCITYAUX_H

#include "AuxKernel.h"

//Forward Declarations
class NSVelocityAux;

template<>
InputParameters validParams<NSVelocityAux>();

/**
 * Velocity auxiliary value
 */
class NSVelocityAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  NSVelocityAux(const std::string & name, InputParameters parameters);

  virtual ~NSVelocityAux() {}

protected:
  virtual Real computeValue();

  VariableValue & _rho;
  VariableValue & _momentum;

};

#endif //VELOCITYAUX_H
