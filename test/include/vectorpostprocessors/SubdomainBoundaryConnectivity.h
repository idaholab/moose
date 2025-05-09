//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class SubdomainBoundaryConnectivity : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  SubdomainBoundaryConnectivity(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override;

protected:
  const MooseMesh & _mesh;
  VectorPostprocessorValue & _connected_ids;
};
