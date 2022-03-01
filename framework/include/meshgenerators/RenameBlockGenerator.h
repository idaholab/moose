//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * MeshGenerator for re-numbering or re-naming blocks
 */
class RenameBlockGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  RenameBlockGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

private:
  /// The old blocks
  std::vector<SubdomainName> _old_block;
  /// The new blocks
  std::vector<SubdomainName> _new_block;
  /// The name of the parameter that specifies the old blocks
  std::string _old_block_param_name;
};
