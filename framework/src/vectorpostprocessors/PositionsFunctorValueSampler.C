//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PositionsFunctorValueSampler.h"
#include "Positions.h"
#include "FEProblemBase.h"

registerMooseObject("MooseApp", PositionsFunctorValueSampler);

InputParameters
PositionsFunctorValueSampler::validParams()
{
  InputParameters params = PointSamplerBase::validParams();
  params.addClassDescription(
      "Sample one or more functors at points specified by a Positions object.");
  params.addRequiredParam<PositionsName>(
      "positions",
      "Positions object specifying the points where you want to evaluate the functors");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "functors", "The functors we want to evaluate at the positions");
  params.addParam<bool>("discontinuous", false, "Indicate that the functors are discontinuous");

  return params;
}

PositionsFunctorValueSampler::PositionsFunctorValueSampler(const InputParameters & parameters)
  : PointSamplerBase(parameters),
    NonADFunctorInterface(this),
    _positions(_fe_problem.getPositionsObject(getParam<PositionsName>("positions")))
{
  const auto & functor_names = getParam<std::vector<MooseFunctorName>>("functors");
  for (const auto & functor_name : functor_names)
    _functors.push_back(&getFunctor<Real>(functor_name));

  _discontinuous_at_faces = getParam<bool>("discontinuous");

  // Initialize the data structures in SamplerBase
  std::vector<std::string> functor_string_names;
  for (const auto & functor_name : functor_names)
    functor_string_names.push_back(functor_name);
  SamplerBase::setupVariables(functor_string_names);
}

void
PositionsFunctorValueSampler::initialize()
{
  // Pull new points
  _points = _positions.getPositions(_fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL);

  PointSamplerBase::initialize();
}

void
PositionsFunctorValueSampler::execute()
{
  BoundingBox bbox = _mesh.getInflatedProcessorBoundingBox();
  const auto state = determineState();

  // Pull new points
  const auto old_size = _points.size();
  _points = _positions.getPositions(_fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL);
  if (_points.size() != old_size)
    mooseError("The points array has grown since initialize() was called");

  for (const auto i : index_range(_points))
  {
    const Point & p = _points[i];

    // Do a bounding box check so we're not doing unnecessary PointLocator lookups
    // In the discontinuous case all ranks must proceed to get a global consensus
    // on who owns face points in getLocalElemContainingPoint()
    if (bbox.contains_point(p) || _discontinuous_at_faces)
    {
      auto & values = _point_values[i];

      if (values.empty())
        values.resize(_functors.size());

      // First find the element the hit lands in
      const Elem * elem = getLocalElemContainingPoint(p);

      if (elem)
      {
        Moose::ElemPointArg elem_point_arg = {elem, p, false};
        for (const auto j : index_range(_functors))
          values[j] = (*_functors[j])(elem_point_arg, state) * _pp_value;

        _found_points[i] = true;
      }
    }
  }
}
