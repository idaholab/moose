//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblem.h"

class SkipContractionTestProblem : public FEProblem
{
public:
  static InputParameters validParams();

  SkipContractionTestProblem(const InputParameters & params);

  bool allowMeshContractionAfterMeshChanged() const override final { return false; }
};
