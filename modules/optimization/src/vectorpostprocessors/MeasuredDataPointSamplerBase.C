//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeasuredDataPointSamplerBase.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariableFE.h"

#include "libmesh/mesh_tools.h"

defineLegacyParams(MeasuredDataPointSamplerBase);

// FIXME LYNN  This is an exact copy of PointSamplerBase because I need to add measurement data to
// the values so that it can be sorted with the values.  This is all because SamplerBase has
// protected inheritance in PointSamplerBase.

InputParameters
MeasuredDataPointSamplerBase::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params += SamplerBase::validParams();

  params.addRequiredCoupledVar(
      "variable", "The names of the variables that this VectorPostprocessor operates on");
  params.addRequiredParam<std::vector<Real>>("measured_data", "Measured data at each point");
  params.addParam<PostprocessorName>(
      "scaling", 1.0, "The postprocessor that the variables are multiplied with");

  return params;
}

MeasuredDataPointSamplerBase::MeasuredDataPointSamplerBase(const InputParameters & parameters)
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
    _measured_data(getParam<std::vector<Real>>("measured_data"))
{
  addMooseVariableDependency(mooseVariable());

  std::vector<std::string> var_names(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    var_names[i] = _coupled_moose_vars[i]->name();

  // fixme lynn should I be worried if a moose var has the same name?
  var_names.push_back("measured_data");

  // Initialize the datastructions in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
MeasuredDataPointSamplerBase::initialize()
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
MeasuredDataPointSamplerBase::execute()
{
  BoundingBox bbox = _mesh.getInflatedProcessorBoundingBox();

  /// So we don't have to create and destroy this
  std::vector<Point> point_vec(1);

  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    Point & p = _points[i];

    // Do a bounding box check so we're not doing unnecessary PointLocator lookups
    if (bbox.contains_point(p))
    {
      auto & values = _point_values[i];

      if (values.empty())
      {
        // fixme lynn everything below assumes a single measurement per point
        unsigned int numberOfMeasuredValuesPerPoint = 1;
        unsigned int numberOfValues = _coupled_moose_vars.size() + numberOfMeasuredValuesPerPoint;
        values.resize(numberOfValues);
      }

      // First find the element the hit lands in
      const Elem * elem = getLocalElemContainingPoint(p);

      if (elem)
      {
        // We have to pass a vector of points into reinitElemPhys
        point_vec[0] = p;

        _subproblem.setCurrentSubdomainID(elem, 0);
        _subproblem.reinitElemPhys(elem, point_vec, 0); // Zero is for tid

        for (MooseIndex(_coupled_moose_vars) j = 0; j < _coupled_moose_vars.size(); ++j)
          values[j] = (dynamic_cast<MooseVariable *>(_coupled_moose_vars[j]))->sln()[0] *
                      _pp_value; // The zero is for the "qp"

        // fixme lynn this is different
        values[_coupled_moose_vars.size()] = _measured_data[i];

        _found_points[i] = true;
      }
    }
  }
}

void
MeasuredDataPointSamplerBase::finalize()
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

  for (MooseIndex(max_id) i = 0; i < max_id.size(); ++i)
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
MeasuredDataPointSamplerBase::getLocalElemContainingPoint(const Point & p)
{
  const Elem * elem = (*_pl)(p);

  if (elem && elem->processor_id() == processor_id())
    return elem;

  return nullptr;
}
