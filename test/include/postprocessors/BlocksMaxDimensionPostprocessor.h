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

/**
 * Gets the mesh dimension of a list of blocks
 */
class BlocksMaxDimensionPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  BlocksMaxDimensionPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

protected:
  /// Subdomain names
  std::vector<SubdomainName> _blocks;
};
