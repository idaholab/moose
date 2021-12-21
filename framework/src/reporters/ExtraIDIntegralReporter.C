//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraIDIntegralReporter.h"

registerMooseObject("MooseApp", ExtraIDIntegralReporter);

InputParameters
ExtraIDIntegralReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addRequiredParam<std::vector<VariableName>>(
      "variable", "The names of the variables that this ExtraIDIntegralReporter operates on");
  params.addRequiredParam<std::vector<ExtraElementIDName>>(
      "id_name", "The list of extra ids name by which to separate integrals.");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of blocks (ids or names) that this object will be applied");
  params.addClassDescription("This ExtraIDIntegralReporter source code is to integrate variables "
                             "based on parsed extra IDs based on reporter system.");
  return params;
}

ExtraIDIntegralReporter::ExtraIDIntegralReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _extra_id_data(declareValueByName<ExtraIDData>("extra_id_data", REPORTER_MODE_REPLICATED))
{
  // add Extra ID Integral VPP
  auto var_names = getParam<std::vector<VariableName>>("variable");
  auto extra_id_names = getParam<std::vector<ExtraElementIDName>>("id_name");
  InputParameters vpp_params =
      _app.getFactory().getValidParams("ExtraIDIntegralVectorPostprocessor");
  vpp_params.set<std::vector<VariableName>>("variable")
      .insert(vpp_params.set<std::vector<VariableName>>("variable").end(),
              var_names.begin(),
              var_names.end());
  vpp_params.set<std::vector<ExtraElementIDName>>("id_name").insert(
      vpp_params.set<std::vector<ExtraElementIDName>>("id_name").end(),
      extra_id_names.begin(),
      extra_id_names.end());
  if (isParamValid("block"))
  {
    auto blocks = getParam<std::vector<SubdomainName>>("block");
    vpp_params.set<std::vector<SubdomainName>>("block").insert(
        vpp_params.set<std::vector<SubdomainName>>("block").end(), blocks.begin(), blocks.end());
  }
  if (isParamValid("execute_on"))
    vpp_params.set<ExecFlagEnum>("execute_on") = getParam<ExecFlagEnum>("execute_on");
  const VectorPostprocessorName vpp_name = _name + "_extra_id_vpp";
  _fe_problem.addVectorPostprocessor("ExtraIDIntegralVectorPostprocessor", vpp_name, vpp_params);
  _vpp_object = &_fe_problem.getUserObject<ExtraIDIntegralVectorPostprocessor>(vpp_name);

  // fill out exta_id_info;
  _extra_id_data.names.insert(
      _extra_id_data.names.end(), extra_id_names.begin(), extra_id_names.end());

  const auto & unique_extra_ids = _vpp_object->getUniqueExtraIds();
  _extra_id_data.unique_ids.resize(unique_extra_ids.size());
  for (unsigned int i = 0; i < unique_extra_ids.size(); ++i)
    _extra_id_data.unique_ids[i].insert(_extra_id_data.unique_ids[i].end(),
                                        (*unique_extra_ids[i]).begin(),
                                        (*unique_extra_ids[i]).end());

  _extra_id_data.integrals = _vpp_object->getIntegrals();
  _extra_id_data.variables.insert(
      _extra_id_data.variables.end(), var_names.begin(), var_names.end());
}

void
to_json(nlohmann::json & json, const ExtraIDIntegralReporter::ExtraIDData & extra_id_data)
{
  auto & info = json["extra_id_data"];
  info["num_id_name"] = extra_id_data.names.size();
  info["id_name"] = extra_id_data.names;
  info["num_values_per_integral"] = extra_id_data.integrals.size();
  info["map_id_to_value"] = extra_id_data.unique_ids;

  auto & integrals = json["integrals"];
  integrals["num_variables"] = extra_id_data.variables.size();
  integrals["variable_names"] = extra_id_data.variables;
  for (unsigned int i = 0; i < extra_id_data.variables.size(); ++i)
  {
    std::string name = extra_id_data.variables[i] + "_integral";
    integrals[name] = (*extra_id_data.integrals[i]);
  }
}
void
dataStore(std::ostream &, ExtraIDIntegralReporter::ExtraIDData &, void *)
{
}

void
dataLoad(std::istream &, ExtraIDIntegralReporter::ExtraIDData &, void *)
{
}
