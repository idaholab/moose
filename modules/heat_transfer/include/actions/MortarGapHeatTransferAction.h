//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "MooseEnum.h"

namespace MortarGapHeatTransfer
{
enum class UserObjectToBuild
{
  CONDUCTION,
  RADIATION,
};

const MultiMooseEnum gapFluxPhysics("conduction radiation");
}

class MortarGapHeatTransferAction : public Action
{
public:
  static InputParameters validParams();

  MortarGapHeatTransferAction(const InputParameters & params);
  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

protected:
  MortarGapHeatTransferAction(const InputParameters & params, bool mortar_enabled);

  static void addMortarParameters(InputParameters & params);

  virtual std::vector<VariableName> temperatureVariables() const;
  virtual BoundaryName primaryBoundary() const;
  virtual BoundaryName secondaryBoundary() const;
  virtual std::vector<BoundaryName> gapFluxBoundaries() const;

  virtual void coreMortarMesh();
  virtual void addConstraints();
  virtual void addMortarMesh();
  virtual void addMortarVariable();
  virtual void addUserObjects();

private:
  void resolveMortarSubdomains();

  const bool _mortar_enabled;
  bool _use_external_mortar_meshes;
  SubdomainName _primary_subdomain;
  SubdomainName _secondary_subdomain;
  const bool _user_provided_gap_flux_models;
  std::vector<MortarGapHeatTransfer::UserObjectToBuild> _gap_flux_models;
};
