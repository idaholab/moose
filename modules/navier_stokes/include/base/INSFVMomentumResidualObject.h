//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "INSFVRhieChowInterpolator.h"

class MooseObject;
class FaceInfo;

/**
 * All objects that contribute to pressure-based (e.g. not density-based) Navier-Stokes momentum
 * equation residuals should inherit from this interface class. This holds true for INSFV, PINSFV,
 * and WCNSFV objects (but not CNSFV or PCNSFV). This interface class introduces virtual methods
 * that are used to gather on-diagonal 'a' coefficient data for Rhie-Chow interpolation
 */
class INSFVMomentumResidualObject
{
public:
  static InputParameters validParams();

  /**
   * @param obj the residual object that inherits from this interface
   */
  template <typename T>
  INSFVMomentumResidualObject(T & obj);

  /**
   * Should be a non-empty implementation if the residual object is a \p FVElementalKernel and
   * introduces residuals that are a function of the velocity, e.g. friction and time-derivative
   * terms.
   */
  virtual void gatherRCData(const Elem & elem) = 0;

  /**
   * Should be a non-empty implementation if the residual object is a \p FVFluxKernel and introduces
   * residuals that are a function of the velocity, e.g. advection, viscosity/diffusion, symmetry
   * boundary conditions, etc.
   */
  virtual void gatherRCData(const FaceInfo & fi) = 0;

  virtual ~INSFVMomentumResidualObject() = default;

protected:
  /// The Rhie-Chow user object that holds the 'a' data structure which we will be feeding
  /// data to
  INSFVRhieChowInterpolator & _rc_uo;

  /// index x|y|z
  const unsigned int _index;
};

template <typename T>
INSFVMomentumResidualObject::INSFVMomentumResidualObject(T & obj)
  : _rc_uo(const_cast<INSFVRhieChowInterpolator &>(
        obj.template getUserObject<INSFVRhieChowInterpolator>("rhie_chow_user_object"))),
    _index(obj.template getParam<MooseEnum>("momentum_component"))
{
}
