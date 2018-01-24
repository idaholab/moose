//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorOfPostprocessors.h"
#include "PostprocessorInterface.h"

template <>
InputParameters
validParams<VectorOfPostprocessors>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "postprocessors", "The postprocessors whose values are to be reported");
  params.addClassDescription("Outputs the values of an arbitrary user-specified set of "
                             "postprocessors as a vector in the order specified by the user");

  return params;
}

VectorOfPostprocessors::VectorOfPostprocessors(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _pp_vec(declareVector(MooseUtils::shortName(parameters.get<std::string>("_object_name"))))
{
  std::vector<PostprocessorName> pps_names(
      getParam<std::vector<PostprocessorName>>("postprocessors"));
  _pp_vec.resize(pps_names.size());
  for (const auto & pps_name : pps_names)
  {
    if (!hasPostprocessorByName(pps_name))
      mooseError(
          "In VectorOfPostprocessors, postprocessor with name: ", pps_name, " does not exist");
    _postprocessor_values.push_back(&getPostprocessorValueByName(pps_name));
  }
}

void
VectorOfPostprocessors::initialize()
{
  _pp_vec.clear();
}

void
VectorOfPostprocessors::execute()
{
  for (const auto & ppv : _postprocessor_values)
    _pp_vec.push_back(*ppv);
}
