/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RANDOMCORROSION_H
#define RANDOMCORROSION_H

// MOOSE includes
#include "AuxKernel.h"
#include "libmesh/mesh_tools.h"

// Forward declarations
class RandomCorrosion;
namespace libMesh
{
namespace MeshTools
{
class BoundingBox;
}
}

template <>
InputParameters validParams<RandomCorrosion>();

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
  MeshTools::BoundingBox _box;

  /// Nodal tolerance for determining if "corrosion" should occur at the current node
  const Real & _nodal_tol;

  /// Minimum  number of "corrosion" points to apply
  const unsigned int & _num_points;

  /// Reference temperature, used for creating a temperature dependence and corrosion
  const Real & _ref_temperature;

  /// System temperature, used for creating a temperature dependence and corrosion
  const PostprocessorValue & _temperature;
};

#endif // RANDOMCORROSION_H
