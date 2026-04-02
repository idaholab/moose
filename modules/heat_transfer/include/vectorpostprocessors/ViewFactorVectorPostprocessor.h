//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class ViewFactorBase;
class GrayLambertSurfaceRadiationBase;

class ViewFactorVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  ViewFactorVectorPostprocessor(const InputParameters & parameters);

  static InputParameters validParams();

  void execute() override;
  void initialize() override;

protected:
  /// The view factor user object
  const ViewFactorBase * const _view_factor_uo;

  /// the surface radiation user object
  const GrayLambertSurfaceRadiationBase * const _glsr_uo;

  /// The surface ids of the data
  VectorPostprocessorValue & _surface_ids;

  /// The data that this VPP harvests off the surface radiation userobject
  std::vector<VectorPostprocessorValue *> _vf;
};
