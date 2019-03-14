//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef THERMALCONTACTACTION_H
#define THERMALCONTACTACTION_H

#include "Action.h"

#include "MooseEnum.h"

class ThermalContactAction : public Action
{
public:
  ThermalContactAction(const InputParameters & params);
  virtual void act() override;

protected:
  virtual void addAuxKernels();
  virtual void addAuxVariables();
  virtual void addBCs();
  virtual void addDiracKernels();
  virtual void addMaterials();
  virtual void addSlaveFluxVector();

  const bool _quadrature;
  const MooseEnum _order;
  const AuxVariableName _penetration_var_name;
  const AuxVariableName _gap_value_name;
  const AuxVariableName _gap_conductivity_name;
};

template <>
InputParameters validParams<ThermalContactAction>();

#endif // THERMALCONTACTACTION_H
