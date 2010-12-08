#ifndef RNDSMOOTHCIRCLEIC_H
#define RNDSMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class RndSmoothCircleIC;

template<>
InputParameters validParams<RndSmoothCircleIC>();

/**
 * SmoothcircleIC just returns a constant value.
 */
class RndSmoothCircleIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  RndSmoothCircleIC(const std::string & name,
                 InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point & p);

private:
  Real _x1;
  Real _y1;
  Real _z1;
  Real _mx_invalue;
  Real _mn_invalue;
  Real _mx_outvalue;
  Real _mn_outvalue;
  Real _radius;

  Point _center;
  
};

#endif //SMOOTHCIRCLEIC_H
