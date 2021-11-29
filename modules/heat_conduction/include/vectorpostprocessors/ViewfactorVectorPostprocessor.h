//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class GrayLambertSurfaceRadiationBase;

class ViewfactorVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  ViewfactorVectorPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

  void execute() override;
  void initialize() override;

protected:
  /// the surface radiation user object
  const GrayLambertSurfaceRadiationBase & _glsr_uo;

  /// The surface ids of the data
  VectorPostprocessorValue & _surface_ids;

  /// The data that this VPP harvests off the surface radiation userobject
  std::vector<VectorPostprocessorValue *> _vf;
};
