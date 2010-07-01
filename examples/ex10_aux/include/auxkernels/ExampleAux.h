#ifndef EXAMPLEAUX_H
#define EXAMPLEAUX_H

#include "AuxKernel.h"


//Forward Declarations
class ExampleAux;

template<>
InputParameters validParams<ExampleAux>();

/** 
 * Coupled auxiliary value
 */
class ExampleAux : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ExampleAux(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~ExampleAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;

  VariableValue & _coupled_val;
};

#endif //EXAMPLEAUX_H
