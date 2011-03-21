#ifndef ELEMENTL2ERROR_H_
#define ELEMENTL2ERROR_H_

#include "ElementIntegral.h"
#include "FunctionInterface.h"

namespace Moose {
  class Function;
}

//Forward Declarations
class ElementL2Error;

template<>
InputParameters validParams<ElementL2Error>();

class ElementL2Error :
  public ElementIntegral,
  public FunctionInterface
{
public:
  ElementL2Error(const std::string & name, InputParameters parameters);
  
  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  Function & _func;
};

#endif //ELEMENTL2ERROR_H_
