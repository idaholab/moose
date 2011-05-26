#ifndef SIDEAVERAGEVALUERZ_H
#define SIDEAVERAGEVALUERZ_H

#include "SideIntegralRZ.h"

//Forward Declarations
class SideAverageValueRZ;

template<>
InputParameters validParams<SideAverageValueRZ>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideAverageValueRZ : public SideIntegralRZ
{
public:
  SideAverageValueRZ(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const Postprocessor & y);
  virtual Real computeIntegral();

protected:
  Real _area;
  Real _current_element_area;
};

#endif
