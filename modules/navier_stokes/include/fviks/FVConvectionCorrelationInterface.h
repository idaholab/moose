//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInterfaceKernel.h"

class FVConvectionCorrelationInterface : public FVInterfaceKernel
{
public:
  static InputParameters validParams();
  FVConvectionCorrelationInterface(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The fluid temperature variable
  const Moose::Functor<ADReal> & _temp_fluid;

  /// The solid/wall temperature variable
  const Moose::Functor<ADReal> & _temp_solid;

  /// The convective heat transfer coefficient in the local element
  const Moose::Functor<ADReal> & _htc;

  /// The distance from the wall before evaluating the bulk temperature
  const Real _bulk_distance;

  /// Whether to use the wall cell for the bulk fluid temperature
  const bool _use_wall_cell;

  /// libmesh object to find points in the mesh
  std::unique_ptr<PointLocatorBase> _pl;
};
