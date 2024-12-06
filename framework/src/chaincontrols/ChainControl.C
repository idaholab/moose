//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControl.h"

InputParameters
ChainControl::validParams()
{
  InputParameters params = Control::validParams();
  params.registerBase("ChainControl");
  return params;
}

ChainControl::ChainControl(const InputParameters & parameters) : Control(parameters) {}

void
ChainControl::addChainControlDataDependency(const std::string & data_name)
{
  if (std::find(_control_data_depends_on.begin(), _control_data_depends_on.end(), data_name) ==
      _control_data_depends_on.end())
    _control_data_depends_on.push_back(data_name);
}

std::string
ChainControl::fullControlDataName(const std::string & data_name, bool apply_object_prefix) const
{
  return (apply_object_prefix ? name() + ":" : "") + data_name;
}
