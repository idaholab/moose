#ifndef SIDEAVERAGEVALUE_H
#define SIDEAVERAGEVALUE_H

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
  SideAverageValue(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

private:
  Real _volume;
};
 
#endif
