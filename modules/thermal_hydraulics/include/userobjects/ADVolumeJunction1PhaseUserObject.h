//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunctionBaseUserObject.h"

class SinglePhaseFluidProperties;
class ADNumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase volume junction
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class ADVolumeJunction1PhaseUserObject : public ADVolumeJunctionBaseUserObject
{
public:
  ADVolumeJunction1PhaseUserObject(const InputParameters & params);

  virtual void finalize() override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Cross-sectional area of connected flow channels
  const ADVariableValue & _A;
  /// rho*A of the connected flow channels
  const ADVariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const ADVariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const ADVariableValue & _rhoEA;

  /// rho*V of the junction
  const ADVariableValue & _rhoV;
  /// rho*u*V of the junction
  const ADVariableValue & _rhouV;
  /// rho*v*V of the junction
  const ADVariableValue & _rhovV;
  /// rho*w*V of the junction
  const ADVariableValue & _rhowV;
  /// rho*E*V of the junction
  const ADVariableValue & _rhoEV;

  /// Flow channel rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// Flow channel rho*u*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// Flow channel rho*E*A coupled variable index
  const unsigned int _rhoEA_jvar;

  /// Junction rho*V coupled variable index
  const unsigned int _rhoV_jvar;
  /// Junction rho*u*V coupled variable index
  const unsigned int _rhouV_jvar;
  /// Junction rho*v*V coupled variable index
  const unsigned int _rhovV_jvar;
  /// Junction rho*w*V coupled variable index
  const unsigned int _rhowV_jvar;
  /// Junction rho*E*V coupled variable index
  const unsigned int _rhoEV_jvar;

  /// Form loss coefficient
  const Real & _K;
  /// Reference area
  const Real & _A_ref;

  /// Single-phase fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Vector of numerical flux user objects for each connected flow channel
  std::vector<const ADNumericalFlux3EqnBase *> _numerical_flux_uo;

public:
  static InputParameters validParams();
};
