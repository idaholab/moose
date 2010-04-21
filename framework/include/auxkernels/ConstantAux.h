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
  ConstantAux(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~ConstantAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;
};

#endif //CONSTANTAUX_H
