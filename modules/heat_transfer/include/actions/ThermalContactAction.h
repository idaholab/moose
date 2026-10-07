//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarGapHeatTransferAction.h"

#include "MooseEnum.h"

class ThermalContactAction : public MortarGapHeatTransferAction
{
public:
  static InputParameters validParams();

  ThermalContactAction(const InputParameters & params);
  virtual void act() override;

protected:
  virtual std::vector<VariableName> temperatureVariables() const override;
  virtual BoundaryName primaryBoundary() const override;
  virtual BoundaryName secondaryBoundary() const override;
  virtual std::vector<BoundaryName> gapFluxBoundaries() const override;

  virtual void addAuxKernels();
  virtual void addAuxVariables();
  virtual void addBCs();
  virtual void addDiracKernels();
  virtual void addMaterials();
  virtual void addSecondaryFluxVector();
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm) override;

  const bool _mortar;
  const bool _quadrature;
  const MooseEnum _order;
  const AuxVariableName _penetration_var_name;
  const AuxVariableName _gap_value_name;
  const AuxVariableName _gap_conductivity_name;

  /// Primary/Secondary boundary name pairs for thermal contact
  const std::vector<std::pair<BoundaryName, BoundaryName>> _boundary_pairs;
  using Action::addRelationshipManagers;
};
