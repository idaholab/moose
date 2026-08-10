//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class CheckOutputFile : public GeneralUserObject
{
public:
  static InputParameters validParams();

  CheckOutputFile(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;

  virtual void postExecute() override;

private:
  /// Name of the Output object whose file will be checked.
  const std::string & _output_object_name;
};
