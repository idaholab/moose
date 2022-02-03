//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AuxKernel.h"
#include "libmesh/bounding_box.h"

/**
 * Creates artificial, temperature driven corrosion.
 *
 * Consider a multi-phase system represented by a field-variable varying
 * from 0 to 1. This class randomly sets points within the field to have
 * a value of 0. Additionally, there is a contrived relationship with the
 * number of points where "corrosion" occurs, the greater the difference
 * between the supplied postprocessor and the reference the more points
 * that are used.
 */
class RandomCorrosion : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters for the RandomCorrosion object.
   */
  RandomCorrosion(const InputParameters & parameters);

  /**
   * At each timestep randomly create a vector of points to apply "corrosion".
   */
  void timestepSetup() override;

protected:
  /**
   * Computes the "corrosion" for the supplied phase variable.
   * @return The compute "phase" variable
   */
  virtual Real computeValue() override;

  /**
   * A helper method for getting random points in the domiain.
   * @return A random point within the bounding box of the domain
   */
  Point getRandomPoint();

private:
  /// The vector of random points to apply "corrosion"
  std::vector<Point> _points;

  /// The bounding box of the domain, used for generating "corrosion" points
  BoundingBox _box;

  /// Nodal tolerance for determining if "corrosion" should occur at the current node
  const Real & _nodal_tol;

  /// Minimum  number of "corrosion" points to apply
  const unsigned int & _num_points;

  /// Reference temperature, used for creating a temperature dependence and corrosion
  const Real & _ref_temperature;

  /// System temperature, used for creating a temperature dependence and corrosion
  const PostprocessorValue & _temperature;
};
