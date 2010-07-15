#ifndef ELEMENTL2ERROR_H
#define ELEMENTL2ERROR_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementL2Error;

template<>
InputParameters validParams<ElementL2Error>();

class ElementL2Error : public ElementIntegral
{
public:
  ElementL2Error(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

private:
  Function & _func;
};

#endif //ELEMENTL2ERROR_H
