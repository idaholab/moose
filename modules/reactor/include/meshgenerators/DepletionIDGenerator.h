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
 * Assigns depletion IDs  to elements based on reporting and material IDs
 */
class DepletionIDGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  DepletionIDGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// input mesh for adding element IDs
  std::unique_ptr<MeshBase> & _input;
  /// material id name
  ExtraElementIDName _material_id_name;
};
