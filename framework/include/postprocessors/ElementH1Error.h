#ifndef ELEMENTH1ERROR_H
#define ELEMENTH1ERROR_H

#include "ElementIntegral.h"
#include "FunctionInterface.h"

class Function;

//Forward Declarations
class ElementH1Error;

template<>
InputParameters validParams<ElementH1Error>();

/**
 * This postprocessor will print out the h1 seminorm between the computed
 * solution and the passed function.
 * ||u,f||h1 is computed as sqrt( (u-f)^2 + (grad u - grad f) * (grad u - grad f) )
 */
class ElementH1Error :
  public ElementIntegral,
  public FunctionInterface
{
public:
  ElementH1Error(const std::string & name, InputParameters parameters);
  
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  Function & _func;
};

#endif //ELEMENTH1ERROR_H
