//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPiecewiseTabularBase.h"

#include "JSONFileReader.h"

InputParameters
KokkosPiecewiseTabularBase::validParams()
{
  InputParameters params = KokkosPiecewiseBase::validParams();
  params += PiecewiseTabularInterface::validParams();

  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the output values");
  params.declareControllable("scale_factor");

  return params;
}

KokkosPiecewiseTabularBase::KokkosPiecewiseTabularBase(const InputParameters & parameters)
  : KokkosPiecewiseBase(parameters),
    PiecewiseTabularInterface(*this, _data_x, _data_y),
    _scale_factor(getParam<Real>("scale_factor"))
{
  // load the data
  if (isParamValid("data_file"))
    buildFromFile(_communicator);
  else if (isParamValid("x") && isParamValid("y"))
    buildFromXandY();
  else if (isParamValid("xy_data"))
    buildFromXY();
  else if (!isParamValid("json_uo"))
    mooseError("Unknown X-Y data source. Are you missing a parameter? Did you misspell one?");
  // JSON data is not available at construction as we rely on a user object
}

void
KokkosPiecewiseTabularBase::initialSetup()
{
  // For JSON UO input, we need to wait for initialSetup to load the data
  if (isParamValid("json_uo"))
    buildFromJSON(getUserObject<JSONFileReader>("json_uo"));

  // Copy data to GPU
  _raw_x = _data_x;
  _raw_y = _data_y;
}
