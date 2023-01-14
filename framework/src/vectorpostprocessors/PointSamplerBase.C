//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointSamplerBase.h"

// MOOSE includes
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/mesh_tools.h"

InputParameters
PointSamplerBase::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params += SamplerBase::validParams();

  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  params.addParam<PostprocessorName>(
      "scaling", 1.0, "The postprocessor that the variables are multiplied with");
  params.addParam<bool>(
      "warn_discontinuous_face_values",
      true,
      "Whether to return a warning if a discontinuous variable is sampled on a face");

  return params;
}

PointSamplerBase::PointSamplerBase(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    SamplerBase(parameters, this, _communicator),
    _mesh(_subproblem.mesh()),
    _pp_value(getPostprocessorValue("scaling")),
    _warn_discontinuous_face_values(getParam<bool>("warn_discontinuous_face_values")),
    _discontinuous_variables(false)
{
  addMooseVariableDependency(&mooseVariableField());

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

  // Check for elemental variables, which are ill-defined on faces for this object
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    if (!_assembly.getFE(_coupled_moose_vars[i]->feType(), _mesh.dimension())->get_continuity())
      _discontinuous_variables = true;
}

void
PointSamplerBase::execute()
{
  BoundingBox bbox = _mesh.getInflatedProcessorBoundingBox();

  /// So we don't have to create and destroy this
  std::vector<Point> point_vec(1);

  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    Point & p = _points[i];

    // Do a bounding box check so we're not doing unnecessary PointLocator lookups
    // If discontinuous variables, do all lookups to have global consensus on point ownership
    if (bbox.contains_point(p) || _discontinuous_variables)
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

        _subproblem.setCurrentSubdomainID(elem, 0);
        _subproblem.reinitElemPhys(elem, point_vec, 0); // Zero is for tid

        for (MooseIndex(_coupled_moose_vars) j = 0; j < _coupled_moose_vars.size(); ++j)
          values[j] = (dynamic_cast<MooseVariableField<Real> *>(_coupled_moose_vars[j]))->sln()[0] *
                      _pp_value; // The zero is for the "qp"

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

  // Consolidate _found_points across processes to know which points were found
  auto _global_found_points = _found_points;
  _comm.sum(_global_found_points);

  // Keep track of maximum process ids for each point to only add it once
  std::vector<unsigned int> max_pid(_found_points.size());
  _comm.maxloc(_found_points, max_pid);

  for (MooseIndex(_found_points) i = 0; i < _found_points.size(); ++i)
  {
    // _global_found_points should contain all 1's at this point (ie every point was found by a
    // proc)
    if (pid == 0 && !_global_found_points[i])
      mooseError("In ", name(), ", sample point not found: ", _points[i]);

    // only process that found the point has the value, and only process with max id should add
    if (pid == max_pid[i] && _found_points[i])
      SamplerBase::addSample(_points[i], _ids[i], _point_values[i]);
  }

  SamplerBase::finalize();
}

void
PointSamplerBase::setPointsVector(const std::vector<Point> & points)
{
  _points = points;
}

void
PointSamplerBase::transferPointsVector(std::vector<Point> && points)
{
  _points = std::move(points);
}

const Elem *
PointSamplerBase::getLocalElemContainingPoint(const Point & p)
{
  const Elem * elem = nullptr;
  if (_discontinuous_variables)
  {
    // Get all possible elements the point may be in
    std::set<const Elem *> candidate_elements;
    (*_pl)(p, candidate_elements);

    // Look at all the element IDs
    std::set<dof_id_type> candidate_ids;
    for (auto candidate : candidate_elements)
      candidate_ids.insert(candidate->id());

    comm().set_union(candidate_ids);

    // Domains without candidate elements will not own the lowest ID one
    if (candidate_elements.size())
    {
      // If we know of the minimum, this will be valid. Otherwise, it'll be
      // nullptr which is fine
      elem = _mesh.queryElemPtr(*(candidate_ids.begin()));

      // Print a warning if it's on a face and a variable is discontinuous
      if (_warn_discontinuous_face_values && candidate_ids.size() > 1)
        mooseDoOnce(mooseWarning("A discontinuous variable is sampled on a face, at ", p));
    }
  }
  else // continuous variables
  {
    // Get all possible elements the point may be in
    // We cant just locate in one element because at the edge between two process domains, it could
    // be that both domains find the element that is not within their domain
    std::set<const Elem *> candidate_elements;
    (*_pl)(p, candidate_elements);

    // Only keep the one that may be local
    for (auto candidate : candidate_elements)
      if (candidate->processor_id() == processor_id())
        elem = candidate;
  }

  if (elem && elem->processor_id() == processor_id())
    return elem;

  return nullptr;
}
