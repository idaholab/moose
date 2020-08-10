//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "nlohmann/json.h"

#include "FileOutput.h"

class JSONOutput : public FileOutput
{
public:
  static InputParameters validParams();
  JSONOutput(const InputParameters & parameters);

protected:
  virtual void output(const ExecFlagType & type) override;
  virtual std::string filename() override;

private:
  nlohmann::json _json;

  // bool _distributed = false;
};
