//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalAuxVariableUserObjectBasePD.h"

/**
 * UserObject class to compute damage index for each material point in PD fracture modeling and
 * simulation
 */
class NodalDamageIndexPD : public NodalAuxVariableUserObjectBasePD
{
public:
  static InputParameters validParams();

  NodalDamageIndexPD(const InputParameters & parameters);

  virtual void computeValue(unsigned int id, dof_id_type dof) override;
};
