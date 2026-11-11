//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "RandomCorrosion.h"
#include "MooseMesh.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("DarcyThermoMechApp", RandomCorrosion);

InputParameters
RandomCorrosion::validParams()
{
  InputParameters params = AuxKernel::validParams();
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
                        "corrosion locations that occurs.");
  params.addParam<PostprocessorName>(
      "temperature",
      274.15,
      "The temperature value to used for computing the temperature "
      "multiplication factor for the number of corrosion locations.");
  return params;
}

RandomCorrosion::RandomCorrosion(const InputParameters & parameters)
  : AuxKernel(parameters),
    _box(MeshTools::create_bounding_box(_mesh)),
    _nodal_tol(getParam<Real>("tolerance")),
    _num_points(getParam<unsigned int>("num_points")),
    _ref_temperature(getParam<Real>("reference_temperature")),
    _temperature(getPostprocessorValue("temperature"))
{
  // This class only works with Nodal aux variables
  if (!isNodal())
    mooseError("RandomCorrosion only operates using nodal aux variables.");
}

void
RandomCorrosion::timestepSetup()
{
  // Increase the number of points as the temperature differs from the reference
  Real factor = 1;
  if (_temperature > _ref_temperature)
    factor = 1 + (_temperature - _ref_temperature) * 0.1;

  // Seed from the master seed and time step so that every processor (and a recovered run)
  // generates the same set of points. The per-node generators of RandomInterface cannot be used
  // here because no node is current during timestepSetup().
  _generator.seed(0, getMasterSeed() + _t_step);

  // Generater the random points to apply "corrosion"
  _points.clear();
  for (unsigned int i = 0; i < _num_points * factor; ++i)
    _points.push_back(getRandomPoint());
}

Real
RandomCorrosion::computeValue()
{

  // If the current node is at a "corrosion" point, set the phase variable to zero
  for (const Point & pt : _points)
    if (_current_node->absolute_fuzzy_equals(pt, _nodal_tol))
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

  Real x = _generator.rand(0) * (max(0) - min(0)) + min(0);
  Real y = _generator.rand(0) * (max(1) - min(1)) + min(1);
  Real z = _generator.rand(0) * (max(2) - min(2)) + min(2);

  return Point(x, y, z);
}
