#ifndef ELEMENTAVERAGEVALUE_H_
#define ELEMENTAVERAGEVALUE_H_

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
  ElementAverageValue(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  Real _volume;
};
 
#endif
