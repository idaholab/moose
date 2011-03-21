#ifndef ELEMENTL2ERROR_H
#define ELEMENTL2ERROR_H

#include "ElementIntegral.h"
#include "FunctionInterface.h"

class Function;

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

#endif //ELEMENTL2ERROR_H
