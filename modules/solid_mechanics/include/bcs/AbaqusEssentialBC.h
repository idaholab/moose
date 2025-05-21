
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusInputObjects.h"
#include "AbaqusUELStepUserObject.h"
#include "DirichletBCBase.h"

class AbaqusUELStepUserObject;

/**
 * Implements Abaqus essential boundary conditions
 */
class AbaqusEssentialBC : public DirichletBCBase
{
public:
  static InputParameters validParams();

  AbaqusEssentialBC(const InputParameters & parameters);

  virtual bool shouldApply() const override;

protected:
  Real computeQpValue() override;

  const AbaqusUELStepUserObject & _step_uo;

  /// Abaqus ID of the current variable
  const Abaqus::AbaqusID _abaqus_var_id;

  /// BC data for the current variable
  // const std::unordered_map<Abaqus::Index, Real> * _node_value_map_previous;
  // const std::unordered_map<Abaqus::Index, Real> & _node_value_map;
};
