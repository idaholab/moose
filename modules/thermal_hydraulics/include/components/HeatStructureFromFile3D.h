//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructureBase.h"
#include "HeatConductionModel.h"

/**
 * Heat structure component that can load the mesh from an ExodusII file
 */
class HeatStructureFromFile3D : public HeatStructureBase
{
public:
  HeatStructureFromFile3D(const InputParameters & params);

  virtual void buildMesh() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

  FunctionName getInitialT() const;

  virtual Real getUnitPerimeter(const HeatStructureSideType & side) const override;

protected:
  virtual std::shared_ptr<HeatConductionModel> buildModel() override;
  virtual void init() override;
  virtual void check() const override;
  virtual bool usingSecondOrderMesh() const override;

  /// The name of the ExodusII file to load the mesh from
  const FileName & _file_name;

public:
  static InputParameters validParams();
};
