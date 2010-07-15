#ifndef ELEMENTH1ERROR_H
#define ELEMENTH1ERROR_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementH1Error;

template<>
InputParameters validParams<ElementH1Error>();

class ElementH1Error : public ElementIntegral
{
public:
  ElementH1Error(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  /**
   * Get the H1 Error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

private:
  Function & _func;
};

#endif //ELEMENTH1ERROR_H
