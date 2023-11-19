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

class MeshDivision;

/**
 * Counts the number of divisions/regions from a MeshDivision object
 */
class NumMeshDivisions : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumMeshDivisions(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  using Postprocessor::getValue;
  virtual Real getValue() const override;

private:
  // MeshDivision object to count the number of divisions/regions from
  const MeshDivision & _mesh_division;
};
