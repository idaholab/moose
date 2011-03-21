#ifndef CONSTANTAUX_H
#define CONSTANTAUX_H

#include "AuxKernel.h"


//Forward Declarations
class ConstantAux;

template<>
InputParameters validParams<ConstantAux>();

/** 
 * Constant auxiliary value
 */
class ConstantAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ConstantAux(const std::string & name, InputParameters parameters);

  virtual ~ConstantAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
};

#endif //CONSTANTAUX_H
