#ifndef PENETRATIONAUX_H
#define PENETRATIONAUX_H

#include "AuxKernel.h"
#include "PenetrationLocator.h"


//Forward Declarations
class PenetrationAux;

template<>
InputParameters validParams<PenetrationAux>();

/** 
 * Constant auxiliary value
 */
class PenetrationAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PenetrationAux(const std::string & name, InputParameters parameters);

  virtual ~PenetrationAux() {}

  virtual void setup();
  
protected:
  virtual Real computeValue();

  PenetrationLocator & _penetration_locator;
};

#endif //PENETRATIONAUX_H
