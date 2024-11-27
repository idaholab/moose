//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVDirichletBCBase.h"

/**
 * A class for turbulent kinetic energy inlet boundary conditions
 */
class INSFVInletIntensityTKEBC : public FVDirichletBCBase
{
public:
  static InputParameters validParams();
  INSFVInletIntensityTKEBC(const InputParameters & params);
  ADReal boundaryValue(const FaceInfo & fi, const Moose::StateArg & state) const override;

protected:
  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// turbulent intensity
  const Moose::Functor<ADReal> & _intensity;

  /// the dimension of the domain
  const unsigned int _dim;
};
