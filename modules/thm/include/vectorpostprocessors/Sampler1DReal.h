#ifndef SAMPLER1DREAL_H
#define SAMPLER1DREAL_H

#include "Sampler1DBase.h"

class Sampler1DReal;

template <>
InputParameters validParams<Sampler1DReal>();

/**
 * This class samples Real material properties for the integration points
 * in all elements in a block of a 1-D mesh.
 */
class Sampler1DReal : public Sampler1DBase<Real>
{
public:
  /**
   * Class constructor
   * Sets up variables for output based on the properties to be output
   * @param parameters The input parameters
   */
  Sampler1DReal(const InputParameters & parameters);

  /**
   * Reduce the material property to a scalar for output
   * In this case, the material property is a Real already, so just return it.
   * @param property The material property
   * @param curr_point The point corresponding to this material property
   * @return A scalar value from this material property to be output
   */
  virtual Real getScalarFromProperty(const Real & property, const Point & curr_point) override;
};

#endif
