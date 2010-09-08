#ifndef RNDBOUNDINGBOXIC_H
#define RNDBOUNDINGBOXIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class RndBoundingBoxIC;

template<>
InputParameters validParams<RndBoundingBoxIC>();

/**
 * RndBoundingBoxIC allows setting the initial condition of a value inside and outside of a specified box.
 * The box is aligned with the x,y,z axis... and is specified by passing in the x,y,z coordinates of the bottom
 * left point and the top right point. Each of the coordinates of the "bottom_left" point MUST be less than
 * those coordinates in the "top_right" point.
 *
 * When setting the initial condition if bottom_left <= Point <= top_right then the "inside" value is used.
 * Otherwise the "outside" value is used.
 */
class RndBoundingBoxIC : public InitialCondition
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  RndBoundingBoxIC(const std::string & name,
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
  Real _mx_inside;
  Real _mx_outside;
  Real _mn_inside;
  Real _mn_outside;
  Real _range_inside;
  Real _range_outside;

  Point _bottom_left;
  Point _top_right;
};

#endif //RNDBOUNDINGBOXIC_H
