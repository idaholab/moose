//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTextureVectorPostprocessor.h"
#include "EulerAngleProvider.h"
#include "Assembly.h"

registerMooseObject("PhaseFieldApp", GrainTextureVectorPostprocessor);

InputParameters
GrainTextureVectorPostprocessor::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params += SamplerBase::validParams();
  params.addClassDescription("Gives out info on the grain boundary properties");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "The EulerAngleProvider User object");
  params.addRequiredCoupledVar("unique_grains", "The grain number");
  params.addRequiredParam<unsigned int>("grain_num", "the number of grains");
  return params;
}

GrainTextureVectorPostprocessor::GrainTextureVectorPostprocessor(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _unique_grains(coupledValue("unique_grains")),
    _grain_num(getParam<unsigned int>("grain_num")),
    _sample(4)
{
  if (_euler.getGrainNum() < _grain_num)
    mooseError("Euler angle provider has too few angles.");

  std::vector<std::string> output_variables(4);
  output_variables[0] = "unique_grain";
  output_variables[1] = "euler_angle_z";
  output_variables[2] = "euler_angle_x\'";
  output_variables[3] = "euler_angle_z\"";
  SamplerBase::setupVariables(output_variables);
}

void
GrainTextureVectorPostprocessor::initialize()
{
  SamplerBase::initialize();
}

void
GrainTextureVectorPostprocessor::execute()
{
  _sample[0] =
      _unique_grains[0] + 1; // Index starts at 0, but we want to display first grain as grain 1.

  const EulerAngles & angle = _euler.getEulerAngles(_unique_grains[0]);
  _sample[1] = angle.phi1; // Get the Z   rotation
  _sample[2] = angle.Phi;  // Get the X'  rotation
  _sample[3] = angle.phi2; // Get the Z'' rotation
  SamplerBase::addSample(_current_elem->vertex_average() /* x,y,z coordinates of elem centroid */,
                         _current_elem->id(),
                         _sample);
}

void
GrainTextureVectorPostprocessor::threadJoin(const UserObject & y)
{
  const GrainTextureVectorPostprocessor & vpp =
      static_cast<const GrainTextureVectorPostprocessor &>(y);
  SamplerBase::threadJoin(vpp);
}

void
GrainTextureVectorPostprocessor::finalize()
{
  SamplerBase::finalize();
}
