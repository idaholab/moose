/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPECIFIEDSMOOTHCIRCLEIC_H
#define SPECIFIEDSMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleBaseIC.h"

// System includes
#include <string>

// Forward Declarations
class SpecifiedSmoothCircleIC;

template<>
InputParameters validParams<SpecifiedSmoothCircleIC>();

/**
 * SpecifiedsmoothCircleIC creates multiple SmoothCircles (number = size of x_positions) that are positioned in the
 * set locations with the set radii.  This is adapted from PolySpecifiedSmoothCircleIC from HYRAX by A.M. Jokisaari
 **/
class SpecifiedSmoothCircleIC : public SmoothCircleBaseIC
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  SpecifiedSmoothCircleIC(const std::string & name,
                         InputParameters parameters);

  virtual void computeCircleRadii();

  virtual void computeCircleCenters();

protected:
  std::vector<Real> _x_positions;
  std::vector<Real> _y_positions;
  std::vector<Real> _z_positions;
  std::vector<Real> _input_radii;
};

#endif //SPECIFIEDSMOOTHCIRCLEIC_H
