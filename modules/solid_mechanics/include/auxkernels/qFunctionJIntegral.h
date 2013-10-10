#ifndef QFUNCTIONJINTEGRAL_H
#define QFUNCTIONJINTEGRAL_H

#include "AuxKernel.h"

/**
 * Coupled auxiliary value
 */
class qFunctionJIntegral : public AuxKernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  qFunctionJIntegral(const std::string & name, InputParameters parameters);

  virtual ~qFunctionJIntegral() {}

protected:
//  virtual Real computeValue(const Point & p);
  virtual Real computeValue();

private:
  const Real _j_integral_radius_inner;
  const Real _j_integral_radius_outer;
  RealVectorValue _crack_location;
};

template<>
InputParameters validParams<qFunctionJIntegral>();

#endif //JINTEGRALDISK_H
