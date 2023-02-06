//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshComponent.h"
#include "HeatStructureInterface.h"

/**
 * Heat structure component that loads the mesh from an ExodusII file
 */
class HeatStructureFromFile3D : public FileMeshComponent, public HeatStructureInterface
{
public:
  static InputParameters validParams();

  HeatStructureFromFile3D(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  /**
   * Has the given region?
   */
  bool hasRegion(const std::string & region) const;

protected:
  virtual bool useCylindricalTransformation() const override { return false; }
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;

  /// Region names
  std::vector<std::string> _region_names;
};
