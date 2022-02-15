//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVFluxBC.h"
#include "INSFVSymmetryBC.h"

/**
 * A class for setting a symmetry boundary condition on the velocity. It should be
 * used in conjunction with an INSFVSymmetryPressureBC.
 */
class INSFVSymmetryVelocityBC : public INSFVFluxBC, public INSFVSymmetryBC
{
public:
  static InputParameters validParams();
  INSFVSymmetryVelocityBC(const InputParameters & params);

  using INSFVFluxBC::gatherRCData;
  void gatherRCData(const FaceInfo & fi) override;

protected:
  /// x-velocity
  const Moose::Functor<ADReal> & _u_functor;
  /// y-velocity
  const Moose::Functor<ADReal> & _v_functor;
  /// z-velocity
  const Moose::Functor<ADReal> & _w_functor;

  /// The dynamic viscosity
  const Moose::Functor<ADReal> & _mu;

  /// The mesh dimension
  const unsigned int _dim;
};
