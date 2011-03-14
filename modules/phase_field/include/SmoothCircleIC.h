#ifndef SMOOTHCIRCLEIC_H
#define SMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class SmoothCircleIC;

template<>
InputParameters validParams<SmoothCircleIC>();

/**
 * SmoothcircleIC just returns a constant value.
 */
class SmoothCircleIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  SmoothCircleIC(const std::string & name,
                 InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

protected:

  Real _x1;
  Real _y1;
  Real _z1;
  Real _invalue;
  Real _outvalue;
  Real _radius;
  Point _center;

};

#endif //SMOOTHCIRCLEIC_H
