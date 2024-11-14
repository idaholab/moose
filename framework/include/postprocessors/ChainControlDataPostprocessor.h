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
#include "ChainControlData.h"

/**
 * Gets a Real or bool chain control value.
 */
class ChainControlDataPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ChainControlDataPostprocessor(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual Real getValue() const override;
  virtual void execute() override;

protected:
  /// Chain control data name
  const std::string & _data_name;
  /// Chain control data if it has type 'Real'
  const ChainControlData<Real> * _real_data;
  /// Chain control data if it has type 'bool'
  const ChainControlData<bool> * _bool_data;
};
