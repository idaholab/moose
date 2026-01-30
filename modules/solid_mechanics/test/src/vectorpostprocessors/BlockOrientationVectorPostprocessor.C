//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BlockOrientationVectorPostprocessor.h"
#include "EulerAngleProvider.h"
#include "Assembly.h"

registerMooseObject("SolidMechanicsTestApp", BlockOrientationVectorPostprocessor);

InputParameters
BlockOrientationVectorPostprocessor::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addRequiredParam<UserObjectName>(
      "euler_angle_provider",
      "The EulerAngleProvider user object that provides Euler angle values in degrees.");
  params.addClassDescription("This object outputs the coordinates, block id, and Euler Angles "
                             "(degrees) associated with each element from a EulerAngleProvider.");
  return params;
}

BlockOrientationVectorPostprocessor::BlockOrientationVectorPostprocessor(
    const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_num(_euler.getGrainNum()),
    _sample(4)
{
  std::vector<std::string> output_variables(4);
  output_variables[0] = "block_id";
  output_variables[1] = "euler_angle_0";
  output_variables[2] = "euler_angle_1";
  output_variables[3] = "euler_angle_2";
  SamplerBase::setupVariables(output_variables);
}

void
BlockOrientationVectorPostprocessor::initialize()
{
  SamplerBase::initialize();
}

void
BlockOrientationVectorPostprocessor::execute()
{
  auto sid = _current_elem->subdomain_id();
  _sample[0] = sid;

  const EulerAngles & angle = _euler.getEulerAngles(sid);
  _sample[1] = angle.phi1;
  _sample[2] = angle.Phi;
  _sample[3] = angle.phi2;
  SamplerBase::addSample(_current_elem->vertex_average() /* x,y,z coordinates of elem centroid */,
                         _current_elem->id(),
                         _sample);
}

void
BlockOrientationVectorPostprocessor::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const BlockOrientationVectorPostprocessor &>(y);
  SamplerBase::threadJoin(vpp);
}

void
BlockOrientationVectorPostprocessor::finalize()
{
  SamplerBase::finalize();
}
