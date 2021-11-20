//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

#include "MooseEnum.h"

class RadiationTransferAction : public Action
{
public:
  RadiationTransferAction(const InputParameters & params);

  static InputParameters validParams();

  virtual void act() override;

protected:
  void addMeshGenerator();
  void addRadiationObject() const;
  void addViewFactorObject() const;
  void addRadiationBCs() const;
  void addRayStudyObject() const;
  void addRayBCs() const;

  std::vector<std::vector<std::string>> radiationPatchNames() const;
  std::vector<std::vector<std::string>> bcRadiationPatchNames() const;
  UserObjectName viewFactorObjectName() const;
  UserObjectName radiationObjectName() const;
  UserObjectName rayStudyName() const;
  std::string rayBCName() const;
  std::string symmetryRayBCName() const;
  MeshGeneratorName meshGeneratorName(unsigned int j) const;

  /// provides the updated number of patches for this boundary
  unsigned int nPatch(unsigned int j) const;

  /// the boundary names participating in the radiative heat transfer
  const std::vector<BoundaryName> _boundary_names;

  /// the type of view factor calculation being performed
  const MooseEnum _view_factor_calculator;
};
