//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointVariableSamplerBase.h"

// MOOSE includes
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/mesh_tools.h"

InputParameters
PointVariableSamplerBase::validParams()
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

PointVariableSamplerBase::PointVariableSamplerBase(const InputParameters & parameters)
  : PointSamplerBase(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)
{
  addMooseVariableDependency(&mooseVariableField());

  std::vector<std::string> var_names(_coupled_moose_vars.size());

  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
  {
    var_names[i] = _coupled_moose_vars[i]->name();
    SamplerBase::checkForStandardFieldVariableType(_coupled_moose_vars[i]);
  }

  // Initialize the data structures in SamplerBase
  SamplerBase::setupVariables(var_names);
}

void
PointVariableSamplerBase::initialize()
{
  SamplerBase::initialize();

  PointSamplerBase::initialize();

  // Check for elemental variables, which are ill-defined on faces for this object
  for (unsigned int i = 0; i < _coupled_moose_vars.size(); i++)
    if (!_assembly.getFE(_coupled_moose_vars[i]->feType(), _mesh.dimension())->get_continuity())
      _discontinuous_at_faces = true;
}

void
PointVariableSamplerBase::execute()
{
  BoundingBox bbox = _mesh.getInflatedProcessorBoundingBox();

  /// So we don't have to create and destroy this
  std::vector<Point> point_vec(1);

  for (MooseIndex(_points) i = 0; i < _points.size(); ++i)
  {
    Point & p = _points[i];

    // Do a bounding box check so we're not doing unnecessary PointLocator lookups
    // In the discontinuous case all ranks must proceed to get a global consensus
    // on who owns face points in getLocalElemContainingPoint()
    if (bbox.contains_point(p) || _discontinuous_at_faces)
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
PointVariableSamplerBase::setPointsVector(const std::vector<Point> & points)
{
  _points = points;
}

void
PointVariableSamplerBase::transferPointsVector(std::vector<Point> && points)
{
  _points = std::move(points);
}
