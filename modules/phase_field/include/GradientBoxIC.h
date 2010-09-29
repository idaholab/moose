#ifndef GradientBOXIC_H
#define GradientBOXIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class GradientBoxIC;

template<>
InputParameters validParams<GradientBoxIC>();

/**
 * GradientBoxIC allows setting the initial condition of a value inside specified box to have a linear slope.
 * The box is aligned with the x,y,z axis... and is specified by passing in the x,y,z coordinates of the bottom
 * left point and the top right point. Each of the coordinates of the "bottom_left" point MUST be less than
 * those coordinates in the "top_right" point.
 */
class GradientBoxIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  GradientBoxIC(const std::string & name,
                MooseSystem & moose_system,
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
  Real _x2; 
  Real _y2; 
  Real _z2; 
  Real _mx_value;
  Real _mn_value;
  
  int _grad_direction;
  int _grad_sign;

  Real _range;

  Point _bottom_left;
  Point _top_right;
};

#endif //GradientBOXIC_H
