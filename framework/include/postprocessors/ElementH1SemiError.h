#ifndef ELEMENTH1SEMIERROR_H_
#define ELEMENTH1SEMIERROR_H_

#include "ElementIntegral.h"
#include "FunctionInterface.h"

namespace Moose {
  class Function;
}

//Forward Declarations
class ElementH1SemiError;

template<>
InputParameters validParams<ElementH1SemiError>();

/**
 * This postprocessor will print out the h1 seminorm between the computed
 * solution and the passed function.
 * ||u,f||h1 is computed as sqrt( (grad u - grad f) * (grad u - grad f) )
 */
class ElementH1SemiError :
  public ElementIntegral,
  public FunctionInterface
{
public:
  ElementH1SemiError(const std::string & name, InputParameters parameters);
  
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();
  Function & _func;
};

#endif //ELEMENTH1SEMIERROR_H_
