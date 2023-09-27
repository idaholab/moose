//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "RadialAverage.h"

class SensitivityFilter : public ElementUserObject
{
public:
  static InputParameters validParams();

  SensitivityFilter(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject &) override{};

protected:
  const RadialAverage::Result & _filter;
  MooseVariable & _density_sensitivity;
  const VariableName _design_density_name;
  const MooseVariable & _design_density;
};
