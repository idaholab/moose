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
#include "PointSamplerBase.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh_tools.h"

template <>
InputParameters
validParams<PointSamplerBase>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params += validParams<SamplerBase>();

  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");

  return params;
}

PointSamplerBase::PointSamplerBase(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    SamplerBase(parameters, this, _communicator),
    _mesh(_subproblem.mesh())
{
  std::vector<std::string> var_names(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // Initialize the datastructions in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
PointSamplerBase::initialize()
{
  SamplerBase::initialize();

  // We do this here just in case it's been destroyed and recreated because of mesh adaptivity.
  _pl = _mesh.getPointLocator();

  // We may not find a requested point on a distributed mesh, and
  // that's okay.
  _pl->enable_out_of_mesh_mode();

  // Reset the point arrays
  _found_points.assign(_points.size(), false);

  _point_values.resize(_points.size());
  std::for_each(
      _point_values.begin(), _point_values.end(), [](std::vector<Real> & vec) { vec.clear(); });
}

void
PointSamplerBase::execute()
{
  MeshTools::BoundingBox bbox = _mesh.getInflatedProcessorBoundingBox();

  /// So we don't have to create and destroy this
  std::vector<Point> point_vec(1);

  for (auto i = beginIndex(_points); i < _points.size(); ++i)
  {
    Point & p = _points[i];

    // Do a bounding box check so we're not doing unnecessary PointLocator lookups
    if (bbox.contains_point(p))
    {
      auto & values = _point_values[i];

      if (values.empty())
        values.resize(_coupled_moose_vars.size());

      // First find the element the hit lands in
      const Elem * elem = getLocalElemContainingPoint(p);

      if (elem)
      {
        // We have to pass a vector of points into reinitElemPhys
        point_vec[0] = p;

        _subproblem.reinitElemPhys(elem, point_vec, 0); // Zero is for tid

        for (auto j = beginIndex(_coupled_moose_vars); j < _coupled_moose_vars.size(); ++j)
          values[j] = _coupled_moose_vars[j]->sln()[0]; // The zero is for the "qp"

        _found_points[i] = true;
      }
    }
  }
}

void
PointSamplerBase::finalize()
{
  // Save off for speed
  const auto pid = processor_id();

  /*
   * Figure out which processor is actually going "claim" each point.
   * If multiple processors found the point and computed values what happens is that
   * maxloc will give us the smallest PID in max_id
   */
  std::vector<unsigned int> max_id(_found_points.size());

  _communicator.maxloc(_found_points, max_id);

  for (auto i = beginIndex(max_id); i < max_id.size(); ++i)
  {
    // Only do this check on the proc zero because it's the same on every processor
    // _found_points should contain all 1's at this point (ie every point was found by a proc)
    if (pid == 0 && !_found_points[i])
      mooseError("In ", name(), ", sample point not found: ", _points[i]);

    if (max_id[i] == pid)
      SamplerBase::addSample(_points[i], _ids[i], _point_values[i]);
  }

  SamplerBase::finalize();
}

const Elem *
PointSamplerBase::getLocalElemContainingPoint(const Point & p)
{
  const Elem * elem = (*_pl)(p);

  if (elem && elem->processor_id() == processor_id())
    return elem;

  return nullptr;
}
