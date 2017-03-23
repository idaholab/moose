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

// MOOSE includes
#include "RandomCorrosion.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<RandomCorrosion>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("tolerance",
                        1e-3,
                        "When acting as a nodal AuxKernel determine if the "
                        "random point to apply corrosion is located at the "
                        "current node.");
  params.addParam<unsigned int>("num_points",
                                10,
                                "The number of random points to apply artificial "
                                "corrosion. The number of points is increased by "
                                "a factor as the supplied temperatures diverge.");
  params.addParam<Real>("reference_temperature",
                        273.15,
                        "Temperature at which corrosion begins, "
                        "the greater the 'temperature' drifts "
                        "from this the greater the amount of "
                        "corrision locations that occurs.");
  params.addParam<PostprocessorName>(
      "temperature",
      274.15,
      "The temperature value to used for computing the temperature "
      "multiplication factor for the number of corrosion locations.");
  return params;
}

RandomCorrosion::RandomCorrosion(const InputParameters & parameters)
  : AuxKernel(parameters),
    _box(MeshTools::bounding_box(_mesh)),
    _nodal_tol(getParam<Real>("tolerance")),
    _num_points(getParam<unsigned int>("num_points")),
    _ref_temperature(getParam<Real>("reference_temperature")),
    _temperature(getPostprocessorValue("temperature"))
{
  // This class only works with Nodal aux variables
  if (!isNodal())
    mooseError("RandomCorrosion only operates using nodal aux variables.");

  // Setup the random number generation
  setRandomResetFrequency(EXEC_TIMESTEP_BEGIN);
}

void
RandomCorrosion::timestepSetup()
{
  // Increase the number of points as the temperature differs from the reference
  Real factor = 1;
  if (_temperature > _ref_temperature)
    factor = 1 + (_temperature - _ref_temperature) * 0.1;

  // Generater the random points to apply "corrosion"
  _points.clear();
  for (unsigned int i = 0; i < _num_points * factor; ++i)
    _points.push_back(getRandomPoint());
}

Real
RandomCorrosion::computeValue()
{

  // If the current node is at a "corrosion" point, set the phase variable to zero
  for (std::vector<Point>::const_iterator it = _points.begin(); it != _points.end(); ++it)
    if (_current_node->absolute_fuzzy_equals(*it, _nodal_tol))
      return 0.0;

  // Do nothing to the phase variable if not at a "corrosion" point
  return _u[_qp];
}

Point
RandomCorrosion::getRandomPoint()
{
  // Generates a random point within the domain
  const Point & min = _box.min();
  const Point & max = _box.max();

  Real x = getRandomReal() * (max(0) - min(0)) + min(0);
  Real y = getRandomReal() * (max(1) - min(1)) + min(1);
  Real z = getRandomReal() * (max(2) - min(2)) + min(2);

  return Point(x, y, z);
}
