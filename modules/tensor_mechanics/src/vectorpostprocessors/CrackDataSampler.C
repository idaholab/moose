//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackDataSampler.h"
#include "PostprocessorInterface.h"
#include "SamplerBase.h"

template <>
InputParameters
validParams<CrackDataSampler>()
{
  InputParameters params = validParams<VectorPostprocessor>();

  params += validParams<SamplerBase>();

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "postprocessors", "The postprocessors whose values are to be reported");
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  MooseEnum position_type("Angle Distance", "Distance");
  params.addParam<MooseEnum>(
      "position_type",
      position_type,
      "The method used to calculate position along crack front.  Options are: " +
          position_type.getRawNames());
  params.addClassDescription("Outputs the values of a set of domain integral postprocessors as a "
                             "vector, along with their positions along the crack front.");

  return params;
}

CrackDataSampler::CrackDataSampler(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _crack_front_definition(&getUserObject<CrackFrontDefinition>("crack_front_definition")),
    _position_type(getParam<MooseEnum>("position_type"))
{
  std::vector<PostprocessorName> pps_names(
      getParam<std::vector<PostprocessorName>>("postprocessors"));
  for (unsigned int i = 0; i < pps_names.size(); ++i)
  {
    if (!hasPostprocessorByName(pps_names[i]))
      mooseError("In CrackDataSampler, postprocessor with name: ", pps_names[i], " does not exist");
    _domain_integral_postprocessor_values.push_back(&getPostprocessorValueByName(pps_names[i]));
  }
  std::vector<std::string> var_names;
  var_names.push_back(name());
  SamplerBase::setupVariables(var_names);
}

void
CrackDataSampler::initialize()
{
  if (_crack_front_definition->getNumCrackFrontPoints() !=
      _domain_integral_postprocessor_values.size())
    mooseError("In CrackDataSampler, number of crack front nodes != number of domain integral "
               "postprocessors");
  if (_position_type == "angle" && !_crack_front_definition->hasAngleAlongFront())
    mooseError(
        "In CrackDataSampler, 'position_type = Angle' specified, but angle is not available.  ",
        "Must specify 'crack_mouth_boundary' in CrackFrontDefinition");
  SamplerBase::initialize();
}

void
CrackDataSampler::execute()
{
  if (processor_id() == 0)
  {
    std::vector<Real> values;
    for (unsigned int i = 0; i < _domain_integral_postprocessor_values.size(); ++i)
    {
      values.clear();
      const Point * crack_front_point = _crack_front_definition->getCrackFrontPoint(i);
      Real position;
      if (_position_type == "Angle")
        position = _crack_front_definition->getAngleAlongFront(i);
      else
        position = _crack_front_definition->getDistanceAlongFront(i);

      values.push_back(*_domain_integral_postprocessor_values[i]);
      addSample(*crack_front_point, position, values);
    }
  }
}

void
CrackDataSampler::finalize()
{
  SamplerBase::finalize();
}
