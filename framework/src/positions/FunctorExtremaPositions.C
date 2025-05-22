//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorExtremaPositions.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/vector_value.h"

registerMooseObject("MooseApp", FunctorExtremaPositions);

InputParameters
FunctorExtremaPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();

  params.addRequiredParam<MooseFunctorName>(
      "functor", "Functor of which the extremas are going to form the list of positions");
  params.addParam<MooseEnum>("extrema_type",
                             MooseEnum("max=0 min=1 max_abs=2", "max"),
                             "Type of extreme value to return. 'max' "
                             "returns the location(s) of the maximum value(s). 'min' returns "
                             "the location(s) of the minimum value(s). 'max_abs' returns the "
                             "locations of the maximum(a) of the absolute value.");
  params.addParam<unsigned int>("num_extrema", 1, "Number of extrema to look for");

  // Future arguments we could conceive:
  // - min_dist to force a certain distance between extrema
  // - functor argument type. We use elements, we could do nodes, qps, sides, ...

  // Use user-provided ordering
  params.set<bool>("auto_sort") = false;
  params.suppressParameter<bool>("auto_sort");
  // The broadcasting has to be custom to preserve the order of the extremas
  params.set<bool>("auto_broadcast") = false;
  params.suppressParameter<bool>("auto_broadcast");

  // Keep as up-to-date as possible given the generality of functors
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription("Searches the geometry for the extrema of the functors");
  return params;
}

FunctorExtremaPositions::FunctorExtremaPositions(const InputParameters & parameters)
  : Positions(parameters),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    _functor(getFunctor<Real>("functor")),
    _n_extrema(getParam<unsigned int>("num_extrema")),
    _type(getParam<MooseEnum>("extrema_type").getEnum<ExtremeType>()),
    _positions_values(declareValueByName<std::vector<Real>, ReporterVectorContext<Real>>(
        "functor_extrema", REPORTER_MODE_REPLICATED))
{
  // Constants and postprocessors are not interesting here
  const auto & functor_name = getParam<MooseFunctorName>("functor");
  if (_fe_problem.hasPostprocessorValueByName(functor_name) ||
      MooseUtils::parsesToReal(functor_name))
    paramError(
        "functor",
        "Postprocessors and constants do not have extrema, they are constant over the domain.");

  if (!_fe_problem.hasFunction(functor_name))
  {
    // Obtain the positions by evaluating the functors
    initialize();
    // Sort if needed (user-specified)
    finalize();
  }
  // Parsed functions are not initialized at construction
  else if (getParam<ExecFlagEnum>("execute_on").contains(EXEC_NONE))
    paramError("functor", "NONE execution flag not supported for a function.");
}

void
FunctorExtremaPositions::initialize()
{
  clearPositions();
  _positions.resize(_n_extrema);
  _positions_values.resize(_n_extrema);

  std::list<Real> extrema;
  std::list<Point> extrema_locs;
  if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
    extrema.resize(_n_extrema, -std::numeric_limits<Real>::max());
  else
    extrema.resize(_n_extrema, std::numeric_limits<Real>::max());
  extrema_locs.resize(_n_extrema);

  const auto time_arg = determineState();

  // Loop over the local mesh, keep track of all extrema
  for (const auto & elem :
       _fe_problem.mesh().getMesh().active_local_subdomain_set_element_ptr_range(blockIDs()))
  {
    const Moose::ElemArg elem_arg = {elem, false};
    auto value = _functor(elem_arg, time_arg);

    auto extremum = extrema.begin();
    auto extremum_loc = extrema_locs.begin();

    for (const auto i : make_range(_n_extrema))
    {
      libmesh_ignore(i);
      bool extrema_found = false;
      switch (_type)
      {
        case ExtremeType::MAX_ABS:
          value = std::abs(value);
          // fallthrough
        case ExtremeType::MAX:
        {
          if (value > *extremum)
            extrema_found = true;
          break;
        }
        case ExtremeType::MIN:
        {
          if (value < *extremum)
            extrema_found = true;
          break;
        }
      }
      if (extrema_found)
      {
        // Move the extrema down the list of extrema
        extrema.insert(extremum, value);
        extrema_locs.insert(extremum_loc, elem->true_centroid());
        extrema.pop_back();
        extrema_locs.pop_back();
        // Found an extremum, filled the extrema vector, done
        break;
      }
      // Consider the next extremum
      std::advance(extremum, 1);
      std::advance(extremum_loc, 1);
    }
  }

  // Synchronize across all ranks
  auto current_candidate = extrema.begin();
  auto current_candidate_loc = extrema_locs.begin();
  for (const auto i : make_range(_n_extrema))
  {
    unsigned int rank;
    // dont modify the extremum, it might be the global n-th extremum in a later call
    auto copy = *current_candidate;
    if (_type == ExtremeType::MAX || _type == ExtremeType::MAX_ABS)
      comm().maxloc(copy, rank);
    else
      comm().minloc(copy, rank);

    RealVectorValue extreme_point(0., 0., 0.);
    if (rank == processor_id())
    {
      extreme_point = *current_candidate_loc;
      // Our candidate for ith max got accepted, move on to offering the next extremal one
      std::advance(current_candidate, 1);
      std::advance(current_candidate_loc, 1);
    }

    // Send the position
    // The broadcast and scatter are for root 0 sending to all. The sum works just as well
    comm().sum(extreme_point);
    _positions[i] = extreme_point;
    _positions_values[i] = copy;
  }
  mooseAssert(comm().verify(_positions_values),
              "Positions extrema should be the same across all MPI processes.");
  _initialized = true;
}
