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

  ///@{ in residual and jacobian setup we check if the area is zero
  void residualSetup() override;
  void jacobianSetup() override;
  ///@}

protected:
  /**
   * check for improper use on an internal face, e.g. for specific postprocessors used if a user
   * imposes this object on an internal face then the 'direction' parameter must be supplied
   */
  void checkForInternalDirection() const;

  /// true if a boundary is an inflow boundary, false if outflow
  virtual bool isInflow() const;

  /// computes the inflow massflux
  ADReal inflowMassFlux(const Moose::StateArg & state) const;

  /// computes the inflow speed
  ADReal inflowSpeed(const Moose::StateArg & state) const;

  /// returns the velocity vector (vel_x, vel_y, vel_z)
  ADRealVectorValue varVelocity(const Moose::StateArg & state) const;

  /// Scaling factor
  const Real _scaling_factor;

  /// Postprocessor with the inlet velocity
  const PostprocessorValue * const _velocity_pp;

  /// Postprocessor with the inlet mass flow rate
  const PostprocessorValue * const _mdot_pp;

  /// Postprocessor with the inlet area
  const PostprocessorValue * const _area_pp;

  /// Fluid density functor
  const Moose::Functor<ADReal> & _rho;

  /// The direction in which the flow is entering/leaving the domain. This is mainly used for cases
  /// when the orientation of the face cannot be established (boundary on an internal face) or when the flow is
  /// entering with an angle compared to the boundary surface.
  const Point _direction;

  /// Flag to store if the flow direction is specified by the user
  const bool _direction_specified_by_user;

  ///@{ Velocity components
  const Moose::Functor<ADReal> & _vel_x;
  const Moose::Functor<ADReal> * const _vel_y;
  const Moose::Functor<ADReal> * const _vel_z;
  ///@}
};

inline void
WCNSFVFluxBCBase::checkForInternalDirection() const
{
  if (_face_info->neighborPtr() && !_direction_specified_by_user)
    mooseError(
        type(),
        " can only be defined on an internal face if the 'direction' parameter is supplied!");
}
