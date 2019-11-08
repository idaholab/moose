//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicNodeMapTester.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", PeriodicNodeMapTester);

InputParameters
PeriodicNodeMapTester::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Test MooseMesh::buildPeriodicNodeMap()");
  params.addCoupledVar("v", "coupled variable (should be periodic)");
  return params;
}

PeriodicNodeMapTester::PeriodicNodeMapTester(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _v_var(coupled("v")),
    _dim(_mesh.dimension()),
    _perf_buildmap(registerTimedSection("buildPeriodicNodeMap", 1))
{
}

void
PeriodicNodeMapTester::initialSetup()
{
  // collect mesh periodicity data
  for (unsigned int i = 0; i < _dim; ++i)
  {
    if (!_mesh.isRegularOrthogonal() || !_mesh.isTranslatedPeriodic(_v_var, i))
      paramError("v", "Variable must be periodic in all directions");

    _periodic_min[i] = _mesh.getMinInDimension(i);
    _periodic_max[i] = _mesh.getMaxInDimension(i);
  }
}

void
PeriodicNodeMapTester::initialize()
{
  // get a fresh point locator
  auto point_locator = _mesh.getPointLocator();

  // Get a pointer to the PeriodicBoundaries buried in libMesh
  auto pbs = _fe_problem.getNonlinearSystemBase().dofMap().get_periodic_boundaries();

  // rebuild periodic node map (this is the heaviest part by far)
  std::multimap<dof_id_type, dof_id_type> periodic_node_map;
  {
    TIME_SECTION(_perf_buildmap);
    _mesh.buildPeriodicNodeMap(periodic_node_map, _v_var, pbs);
  }

  // analyze map
  unsigned int n_corner = 0;
  for (auto item : periodic_node_map)
  {
    Node * node_ptr = _mesh.nodePtr(item.first);
    if (node_ptr == nullptr)
      mooseError("Non-existing node found in periodic_node_map");

    Point p(*node_ptr);
    bool corner = true;
    for (unsigned int i = 0; i < _dim; ++i)
      corner = corner && (MooseUtils::absoluteFuzzyEqual(p(i), _periodic_min[i]) ||
                          MooseUtils::absoluteFuzzyEqual(p(i), _periodic_max[i]));
    if (corner)
      n_corner++;
  }

  // we need 2 periodicity entries in 1D
  if (_dim == 1 && n_corner != 2)
    mooseWarning("Expected 2 entries in 1D, but found ", n_corner);

  // we need 8 periodicity entries in 2D
  if (_dim == 2 && n_corner != 8)
    mooseWarning("Expected 8 entries in 2D, but found ", n_corner);

  // we need 24 periodicity entries in 3D
  if (_dim == 3 && n_corner != 24)
    mooseWarning("Expected 24 entries in 2D, but found ", n_corner);
}

void
PeriodicNodeMapTester::execute()
{
}

void
PeriodicNodeMapTester::finalize()
{
}
