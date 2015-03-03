/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHCIRCLEBASEIC_H
#define SMOOTHCIRCLEBASEIC_H

#include "Kernel.h"
#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class SmoothCircleBaseIC;

template<>
InputParameters validParams<SmoothCircleBaseIC>();

/**
 * SmoothcircleBaseIC is the base class for all initial conditions that create circles. The circles can have sharp interfaces or a finite interface width. Note that all children must resize _radii and _centers.
 */
class SmoothCircleBaseIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  SmoothCircleBaseIC(const std::string & name,
                 InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */

protected:
  MooseMesh & _mesh;

  Real _invalue;
  Real _outvalue;
  Real _int_width;
  bool _3D_spheres;

  unsigned int _num_dim;

  std::vector<Point> _centers;
  std::vector<Real> _radii;

  virtual void initialSetup();

  virtual Real value(const Point & p);

  virtual RealGradient gradient(const Point & p);

  virtual Real computeCircleValue(const Point & p, const Point & center, const Real & radius);

  virtual Point computeCircleGradient(const Point & p, const Point & center, const Real & radius);

  virtual void computeCircleRadii() = 0;

  virtual void computeCircleCenters() = 0;
};

#endif //SMOOTHCIRCLEBASEIC_H
