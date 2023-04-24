//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingularTripletReporter.h"
#include "JsonIO.h"

registerMooseObject("StochasticToolsApp", SingularTripletReporter);

InputParameters
SingularTripletReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += MappingInterface::validParams();

  params.addClassDescription("Tool for accessing and outputting the singular triplets of a "
                             "singular value decomposition in PODMapping.");

  params.addRequiredParam<UserObjectName>(
      "pod_mapping", "The PODMapping object whose singular triplets should be printed.");

  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "The names of the variables whose SVD should be printed.");

  return params;
}

SingularTripletReporter::SingularTripletReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    MappingInterface(this),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _left_singular_vectors(
        declareValueByName<std::map<VariableName, std::vector<DenseVector<Real>>>>(
            "left_singular_vectors", REPORTER_MODE_ROOT)),
    _right_singular_vectors(
        declareValueByName<std::map<VariableName, std::vector<DenseVector<Real>>>>(
            "right_singular_vectors", REPORTER_MODE_ROOT)),
    _singular_values(declareValueByName<std::map<VariableName, std::vector<Real>>>(
        "singular_values", REPORTER_MODE_ROOT))

{
  for (const auto & vname : _variable_names)
  {
    _left_singular_vectors.emplace(vname, std::vector<DenseVector<Real>>());
    _right_singular_vectors.emplace(vname, std::vector<DenseVector<Real>>());
    _singular_values.emplace(vname, std::vector<Real>());
  }
}

void
SingularTripletReporter::initialSetup()
{
  _pod_mapping = dynamic_cast<PODMapping *>(&getMapping("pod_mapping"));

  if (!_pod_mapping)
    paramError("pod_mapping", "The given mapping is not a PODMapping!");

  const auto & pod_mapping_variables = _pod_mapping->getVariableNames();
  for (const auto & vname : _variable_names)
    if (std::find(pod_mapping_variables.begin(), pod_mapping_variables.end(), vname) ==
        pod_mapping_variables.end())
      paramError("variables",
                 "The SVD of the requested variable " + vname +
                     " is not in the PODMapping object!");

  for (const auto & vname : _variable_names)
  {
    _left_singular_vectors[vname].clear();
    _right_singular_vectors[vname].clear();
    _singular_values[vname].clear();
  }
}

void
SingularTripletReporter::execute()
{
  for (const auto & vname : _variable_names)
  {
    _pod_mapping->buildMapping(vname);
    _singular_values[vname] = _pod_mapping->singularValues(vname);

    const auto & left_modes = _pod_mapping->leftBasis(vname);
    const auto & right_modes = _pod_mapping->rightBasis(vname);

    for (auto mode_i : index_range(left_modes))
    {
      _left_singular_vectors[vname].push_back(left_modes[mode_i]);
      _right_singular_vectors[vname].push_back(right_modes[mode_i]);
    }
  }
}
