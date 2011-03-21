#ifndef SIDEAVERAGEVALUE_H_
#define SIDEAVERAGEVALUE_H_

#include "SideIntegral.h"

//Forward Declarations
class SideAverageValue;

template<>
InputParameters validParams<SideAverageValue>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideAverageValue : public SideIntegral
{
public:
  SideAverageValue(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  Real _volume;
};
 
#endif
