//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NS.h"
#include "FVFluxBC.h"
#include "INSFVFlowBC.h"

/**
 * Base class for weakly compressible flux boundary conditions
 */
class WCNSFVFluxBCBase : public FVFluxBC, public INSFVFlowBC
{
public:
  static InputParameters validParams();
  WCNSFVFluxBCBase(const InputParameters & params);

protected:
  /**
   * check for improper use on an internal face, e.g. for specific postprocessors used if a user
   * imposes this object on an internal face then the 'direction' parameter must be supplied
   */
  void checkForInternalDirection() const;

  /// Scaling factor
  const Real _scaling_factor;

  /// Postprocessor with the inlet velocity
  const PostprocessorValue * const _velocity_pp;

  /// Postprocessor with the inlet mass flow rate
  const PostprocessorValue * const _mdot_pp;

  /// Postprocessor with the inlet area
  const PostprocessorValue * const _area_pp;

  /// Fluid density functor
  const Moose::Functor<ADReal> * const _rho;

  /// The direction in which the flow is entering/leaving the domain. This is mainly used for cases
  /// when the orientation of the face cannot be established (boundary on an internal face) or when the flow is
  /// entering with an angle compared to the boundary surface.
  const Point _direction;

  /// Flag to store if the flow direction is specified by the user
  const bool _direction_specified_by_user;
};

inline void
WCNSFVFluxBCBase::checkForInternalDirection() const
{
  if (_face_info->neighborPtr() && !_direction_specified_by_user)
    mooseError(
        type(),
        " can only be defined on an internal face if the 'direction' parameter is supplied!");
}
