#ifndef CONSTANTDAMPER_H
#define CONSTANTDAMPER_H

// Moose Includes
#include "Damper.h"

//Forward Declarations
class ConstantDamper;

template<>
InputParameters validParams<ConstantDamper>();

class ConstantDamper : public Damper
{
public:
  ConstantDamper(std::string name, MooseSystem & moose_system, InputParameters parameters);

protected:
  /**
   * This MUST be overriden by a child ConstantDamper.
   *
   * This is where they actually compute a number between 0 and 1.
   */
  virtual Real computeQpDamping();

  /**
   * The constant amount of the newton update to take.
   */
  Real _damping;
};
 
#endif //CONSTANTDAMPER_H
