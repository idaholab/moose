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
 * that are used to gather data for Rhie-Chow interpolation and for interpolating and reconstructing
 * body forces
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
   * Should be a non-empty implementation if the residual object is a \p FVElementalKernel and is
   * not a pressure gradient term, e.g. this should be a non-empty implementation if the residual
   * object introduces body forces. This will contribute to 'B' data structures where 'B' represents
   * body forces in Moukalled notation
   */
  virtual void gatherRCData(const Elem & elem) = 0;

  /**
   * Should be a non-empty implementation if the residual object is a \p FVFluxKernel. This will
   * contribute to 'a' data structures where 'a' represents on-diagonal momentum equation
   * coefficients in Moukalled notation
   */
  virtual void gatherRCData(const FaceInfo & fi) = 0;

  virtual ~INSFVMomentumResidualObject() = default;

protected:
  /// The Rhie-Chow user object that holds the 'a' and 'B' data structures which we will be feeding
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
