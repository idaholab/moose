#ifndef SIDEINTEGRAL_H
#define SIDEINTEGRAL_H

#include "SidePostprocessor.h"

//Forward Declarations
class SideIntegral;

template<>
InputParameters validParams<SideIntegral>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegral : public SidePostprocessor
{
public:
  SideIntegral(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

private:
  Real _integral_value;
};
 
#endif
