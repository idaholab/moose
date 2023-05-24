//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileOutput.h"
#include "MappingInterface.h"
#include "VariableMappingBase.h"

/**
 * Class which is used to output valuable data in binary format from Mapping objects
 */
class MappingOutput : public FileOutput, public MappingInterface
{
public:
  static InputParameters validParams();
  MappingOutput(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type) override;

private:
  /// List of supplied Mapping objects
  const std::vector<std::string> & _mappings;
};
