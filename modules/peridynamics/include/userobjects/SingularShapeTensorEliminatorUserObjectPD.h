//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObjectBasePD.h"

/**
 * UserObject class to eliminate the existance of singular shape tensor due to bond breakage
 * determined by a bond failure criterion.
 * Singularity is checked for all bonds, and bonds with singular shape tensor are treated
 * as broken as if determined by a failure criterion. Due to nonlocal dependency in a shape tensor
 * calculation, the above procesess is repeated with updated bond_status value from previous step
 * untill no further shape tensor singularity is detected. Shape tensor singularity is not allowed
 * in MOOSE solve.
 */
class SingularShapeTensorEliminatorUserObjectPD : public GeneralUserObjectBasePD
{
public:
  static InputParameters validParams();

  SingularShapeTensorEliminatorUserObjectPD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// function to compute and check the singularity of shape tensor of a bond
  bool checkShapeTensorSingularity(const Elem * elem);
};
