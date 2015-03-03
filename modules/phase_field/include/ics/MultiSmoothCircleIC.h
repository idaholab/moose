/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTISMOOTHCIRCLEIC_H
#define MULTISMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleBaseIC.h"

// System includes
#include <string>

// Forward Declarations
class MultiSmoothCircleIC;

template<>
InputParameters validParams<MultiSmoothCircleIC>();

/**
 * MultismoothCircleIC creates multiple SmoothCircles (number = numbub) that are randomly
 * positioned around the domain, with a minimum spacing equal to bubspac
 **/
class MultiSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  MultiSmoothCircleIC(const std::string & name,
                      InputParameters parameters);

  virtual void initialSetup();

  virtual void computeCircleRadii();

  virtual void computeCircleCenters();

protected:

  unsigned int _numbub;
  Real _bubspac;

  unsigned int _numtries;
  Real _radius;
  Real _radius_variation;
  MooseEnum _radius_variation_type;

  Point _bottom_left;
  Point _top_right;
  Point _range;
};

#endif //MULTISMOOTHCIRCLEIC_H
