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
  ExampleAux(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as);

  virtual ~ExampleAux() {}
  
protected:
  virtual Real computeValue();

  Real _value;

  Real & _coupled_val;
};

#endif //EXAMPLEAUX_H
