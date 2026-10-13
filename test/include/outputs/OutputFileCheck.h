//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Output.h"
#include "FileOutput.h"

class OutputFileCheck;

class OutputFileCheck : public Output
{
public:
  static InputParameters validParams();

  OutputFileCheck(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual void output() override;

private:
  /// Name of the Output object to dependency
  const std::string & _output_object_name;
};
