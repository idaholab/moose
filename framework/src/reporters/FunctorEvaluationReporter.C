//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorPointEvaluationReporter.h"

registerMooseObject("MooseApp", FunctorPointEvaluationReporter);

InputParameters
FunctorPointEvaluationReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += NonADFunctorInterface::validParams();
  params.addClassDescription("Reporter of functor values evaluated at the user-requested points to "
                             "be accessed by other objects.");

  // Many types are conceivable here, as themselves, vectors, vectors of vectors... as needed

  // Reporters holding a single Real
  params.addParam<std::vector<std::string>>("real_reporter_names",
                                            "The names of the `Real`-valued reporters to declare");
  params.addParam<std::vector<MooseFunctorName>>(
      "real_reporter_functors", "The functors to evaluate to fill the `Real`-valued reporters");
  params.addParam<std::vector<Point>>(
      "real_reporter_evaluation_points",
      "The points to evaluate the functors at to fill the `Real`-valued reporters");

  // Reporters holding a std::vector<Real>
  params.addParam<std::vector<std::string>>(
      "real_vector_reporter_names",
      "The names of the `std::vector<Real>`-valued (size 3) reporters to declare");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "real_vector_reporter_functors",
      "The real-valued functors to evaluate to fill the `std::vector<Real>`-valued reporters");
  params.addParam<std::vector<Point>>(
      "real_vector_reporter_evaluation_points",
      "The points to evaluate the functors at to fill the `std::vector<Real>`-valued reporters");

  // Reporters holding a std::vector<Point>
  params.addParam<std::vector<std::string>>(
      "point_vector_reporter_names",
      "The names of the `std::vector<Point>`-valued reporters to declare");
  params.addParam<std::vector<std::vector<MooseFunctorName>>>(
      "point_vector_reporter_functors",
      "The RealVectorValue-valued functors to evaluate to fill the `std::vector<Point>`-valued "
      "reporters");
  params.addParam<std::vector<Point>>(
      "point_vector_reporter_evaluation_points",
      "The points to evaluate the functors at to fill the `std::vector<Point>`-valued reporters");

  return params;
}

FunctorPointEvaluationReporter::FunctorPointEvaluationReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    NonADFunctorInterface(this),
    _real_reporter_names(getParam<std::vector<std::string>>("real_reporter_names")),
    _real_vector_reporter_names(getParam<std::vector<std::string>>("real_vector_reporter_names")),
    _point_vector_reporter_names(getParam<std::vector<std::string>>("point_vector_reporter_names")),
    _real_reporter_points(getParam<std::vector<Point>>("real_reporter_evaluation_points")),
    _real_vector_reporter_points(
        getParam<std::vector<Point>>("real_vector_reporter_evaluation_points")),
    _point_vector_reporter_points(
        getParam<std::vector<Point>>("point_vector_reporter_evaluation_points"))
{
  // Parameter checking
  if (_real_reporter_names.size() != _real_reporter_points.size())
    paramError("real_reporter_names",
               "'real_reporter_names' (" + std::to_string(_real_reporter_names.size()) +
                   ") must be the same size as 'real_reporter_evaluation_points' (" +
                   std::to_string(_real_reporter_points.size()) + ")");
  if (_real_vector_reporter_names.size() != _real_vector_reporter_points.size())
    paramError("real_vector_reporter_names",
               "'real_vector_reporter_names' (" + std::to_string(_real_reporter_names.size()) +
                   ") must be the same size as 'real_vector_reporter_evaluation_points' (" +
                   std::to_string(_real_reporter_points.size()) + ")");
  if (_point_vector_reporter_names.size() != _point_vector_reporter_points.size())
    paramError("point_vector_reporter_names",
               "'point_vector_reporter_names' (" + std::to_string(_real_reporter_names.size()) +
                   ") must be the same size as 'point_vector_reporter_evaluation_points' (" +
                   std::to_string(_real_reporter_points.size()) + ")");

  // Declare all reporters
  for (const auto & name : _real_reporter_names)
    _real_reporter_values.push_back(declareValueByName<Real>(name, REPORTER_MODE_DISTRIBUTED));
  for (const auto & name : _real_vector_reporter_names)
    _real_vector_reporter_values.push_back(
        declareValueByName<std::vector<Real>>(name, REPORTER_MODE_DISTRIBUTED));
  for (const auto & name : _point_vector_reporter_names)
    _point_vector_reporter_values.push_back(declareValueByName<std::vector<Point>>(
        name, REPORTER_MODE_DISTRIBUTED, std::vector<Point>(3)));

  // Retrieve the functors
  for (const auto & functor_name :
       getParam<std::vector<MooseFunctorName>>("real_reporter_functors"))
    _real_reporter_functors.push_back(&getFunctor<Real>(functor_name));
  for (const auto & functor_vec :
       getParam<std::vector<std::vector<MooseFunctorName>>>("real_vector_reporter_functors"))
  {
    _real_vector_reporter_functors.push_back(std::vector<const Moose::Functor<Real> *>());
    for (const auto & functor_name : functor_vec)
      _real_vector_reporter_functors.rbegin()->push_back(&getFunctor<Real>(functor_name));
  }
  for (const auto & functor_vec :
       getParam<std::vector<std::vector<MooseFunctorName>>>("point_vector_reporter_functors"))
  {
    _point_vector_reporter_functors.push_back(std::vector<const Moose::Functor<Real> *>());
    for (const auto & functor_name : functor_vec)
      _point_vector_reporter_functors.rbegin()->push_back(&getFunctor<Real>(functor_name));
  }

  // Size the reporter vectors for the number of functors to use
  for (const auto i : index_range(_real_vector_reporter_values))
    _real_vector_reporter_values[i].get().resize(_real_vector_reporter_functors[i].size());
  for (const auto i : index_range(_point_vector_reporter_values))
    _point_vector_reporter_values[i].get().resize(_point_vector_reporter_functors[i].size() / 3);

  _pl = _subproblem.mesh().getPointLocator();
  _pl->enable_out_of_mesh_mode();
}

void
FunctorPointEvaluationReporter::execute()
{
  const Moose::StateArg time = Moose::currentState();

  // Fill the reporters, only if the point is local
  for (auto i : index_range(_real_reporter_points))
  {
    const auto * elem = (*_pl)(_real_reporter_points[i]);
    // only consider points inside the local domain
    if (!elem || elem->processor_id() != processor_id())
      continue;

    // let's start without skewness correction to avoid ghosting concerns
    Moose::ElemPointArg point_arg = {elem, _real_reporter_points[i], false};
    _real_reporter_values[i].get() = (*_real_reporter_functors[i])(point_arg, time);
  }

  // same for vector<Real> reporters
  for (auto i : index_range(_real_reporter_points))
  {
    const auto * elem = (*_pl)(_real_reporter_points[i]);
    if (!elem || elem->processor_id() != processor_id())
      continue;

    // let's start without skewness correction to avoid ghosting concerns
    Moose::ElemPointArg point_arg = {elem, _real_reporter_points[i], false};

    for (auto j : index_range(_real_vector_reporter_functors[i]))
      _real_vector_reporter_values[i].get()[j] =
          (*_real_vector_reporter_functors[i][j])(point_arg, time);
  }

  // same for vector<Point> reporters
  for (auto i : index_range(_point_vector_reporter_points))
  {
    const auto * elem = (*_pl)(_real_reporter_points[i]);
    if (!elem || elem->processor_id() != processor_id())
      continue;

    // let's start without skewness correction to avoid ghosting concerns
    Moose::ElemPointArg point_arg = {elem, _real_reporter_points[i], false};

    for (auto j = 0; j < 3; j++)
      _point_vector_reporter_values[i].get()[j] =
          (*_point_vector_reporter_functors[i][j])(point_arg, time);
  }
}
