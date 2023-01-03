//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class VectorPostprocessorReductionValue : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  VectorPostprocessorReductionValue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  /// Name of the VectorPostprocessor object that contains the vector to read
  const VectorPostprocessorName _vpp_name;
  /// Name of the vector to read the component from
  const std::string _vector_name;
  /// VectorPostprocessorValue object to read a specified component from
  const VectorPostprocessorValue & _vpp_values;
  /// The type pf reduction operation performed on the vpp
  const MooseEnum _value_type;
};
