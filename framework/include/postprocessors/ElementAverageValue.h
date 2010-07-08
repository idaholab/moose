#ifndef ELEMENTAVERAGEVALUE_H
#define ELEMENTAVERAGEVALUE_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementAverageValue;

template<>
InputParameters validParams<ElementAverageValue>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementAverageValue : public ElementIntegral
{
public:
  ElementAverageValue(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

private:
  Real _volume;
};
 
#endif
