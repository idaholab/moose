#ifndef ELEMENTINTEGRALRZ_H
#define ELEMENTINTEGRALRZ_H

#include "ElementIntegral.h"

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegralRZ : public ElementIntegral
{
public:
  ElementIntegralRZ(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpIntegral();

};

template<>
InputParameters validParams<ElementIntegralRZ>();

#endif
