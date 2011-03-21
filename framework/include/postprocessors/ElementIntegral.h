#ifndef ELEMENTINTEGRAL_H_
#define ELEMENTINTEGRAL_H_

#include "ElementPostprocessor.h"

//Forward Declarations
class ElementIntegral;

template<>
InputParameters validParams<ElementIntegral>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegral : public ElementPostprocessor
{
public:
  ElementIntegral(const std::string & name, InputParameters parameters);
  
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

  Real _integral_value;
};
 
#endif
