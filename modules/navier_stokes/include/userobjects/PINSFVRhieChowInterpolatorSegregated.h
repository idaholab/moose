//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVRhieChowInterpolatorSegregated.h"

/**
 * A user object which implements the Rhie Chow interpolation for segregated
 * porous medium momentum-pressure systems.
 */
class PINSFVRhieChowInterpolatorSegregated : public INSFVRhieChowInterpolatorSegregated
{
public:
  static InputParameters validParams();
  PINSFVRhieChowInterpolatorSegregated(const InputParameters & params);

protected:
  const Moose::FunctorBase<ADReal> & epsilon(THREAD_ID tid) const override;

  /// The thread 0 copy of the porosity functor held by the subproblem. Initially this functor
  /// should be provided by a functor material property or function. We then perform repeated
  /// interpolations and reconstructions to create the resulting smoothed field
  const Moose::Functor<ADReal> & _eps;

  /// All the thread copies of the problem's porosity functor
  std::vector<const Moose::Functor<ADReal> *> _epss;
};

inline const Moose::FunctorBase<ADReal> &
PINSFVRhieChowInterpolatorSegregated::epsilon(const THREAD_ID tid) const
{
  return *_epss[tid];
}
