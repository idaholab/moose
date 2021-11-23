//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationData.h"

registerMooseObject("isopodApp", OptimizationData);

InputParameters
OptimizationData::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter to hold data transferred between optimization driver, "
                             "forward and adjoint problems");
  return params;
}

OptimizationData::OptimizationData(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _optimization_data(declareValueByName<std::vector<std::tuple<Point, Real, Real, Real>>>(
        "optimization_data", REPORTER_MODE_REPLICATED))
{
}

void
OptimizationData::execute()
{
}

// fixme I can't put this here because ReproterPointSource2 can't see it
namespace libMesh
{
void
to_json(nlohmann::json & json,
        const std::vector<std::tuple<Point, Real, Real, Real>> & optimization_data)
{
  Point measurement_point;
  Real measurement_value;
  Real simulation_value;
  Real misfit_value;

  for (auto & v : optimization_data)
  {
    std::stringstream ss;
    std::tie(measurement_point, measurement_value, simulation_value, misfit_value) = v;
    ss << "(";
    for (const auto & i : make_range(LIBMESH_DIM))
    {
      ss << measurement_point(i);
      if (i < (LIBMESH_DIM - 1))
        ss << ", ";
    }
    ss << ")";
    json["measurement_point"].push_back(ss.str());
    json["measurement_value"].push_back(measurement_value);
    json["simualtion_value"].push_back(simulation_value);
    json["misfit_value"].push_back(misfit_value);
  }
}
}

template <>
void
dataStore(std::ostream & stream,
          std::tuple<Point, Real, Real, Real> & optimization_data_entry,
          void * context)
{
  Point measurement_point;
  Real measurement_value;
  Real simualtion_value;
  Real misfit_value;
  std::tie(measurement_point, measurement_value, simualtion_value, misfit_value) =
      optimization_data_entry;
  dataStore(stream, measurement_point, context);
  dataStore(stream, measurement_value, context);
  dataStore(stream, simualtion_value, context);
  dataStore(stream, misfit_value, context);
};

template <>
void
dataLoad(std::istream & stream,
         std::tuple<Point, Real, Real, Real> & optimization_data_entry,
         void * context)
{
  Point measurement_point;
  Real measurement_value;
  Real simualtion_value;
  Real misfit_value;
  std::tie(measurement_point, measurement_value, simualtion_value, misfit_value) =
      optimization_data_entry;
  dataLoad(stream, measurement_point, context);
  dataLoad(stream, measurement_value, context);
  dataLoad(stream, simualtion_value, context);
  dataLoad(stream, misfit_value, context);
};
