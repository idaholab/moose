#ifndef SMOOTHCIRCLEIC_H
#define SMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class SmoothCircleIC;

template<>
InputParameters validParams<SmoothCircleIC>();

/**
 * SmoothcircleIC creates a circle of a given radius centered at a given point in the domain.
 * If int_width > zero, the border of the circle with smoothly transition from
 * the invalue to the outvalue.
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

  virtual RealGradient gradient(const Point & p);

protected:
  MooseMesh & _mesh;

  Real _x1;
  Real _y1;
  Real _z1;
  Real _invalue;
  Real _outvalue;
  Real _radius;
  Real _int_width;
  bool _3D_spheres;
  Point _center;

  unsigned int _num_dim;
};

#endif //SMOOTHCIRCLEIC_H
