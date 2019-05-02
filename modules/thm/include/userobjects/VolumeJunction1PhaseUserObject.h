#pragma once

#include "VolumeJunctionBaseUserObject.h"

class VolumeJunction1PhaseUserObject;
class SinglePhaseFluidProperties;
class NumericalFlux3EqnBase;

template <>
InputParameters validParams<VolumeJunction1PhaseUserObject>();

/**
 * Computes and caches flux and residual vectors for a 1-phase volume junction
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class VolumeJunction1PhaseUserObject : public VolumeJunctionBaseUserObject
{
public:
  VolumeJunction1PhaseUserObject(const InputParameters & params);

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /**
   * Returns a vector of coupled flow variable names, indexed by equation index
   */
  static std::vector<std::string> getCoupledFlowVariableNames();

  /**
   * Returns a vector of coupled scalar variable names, indexed by equation index
   */
  static std::vector<std::string> getCoupledScalarVariableNames();

  /// Cross-sectional area of connected flow channels
  const VariableValue & _A;
  /// rho*A of the connected flow channels
  const VariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const VariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const VariableValue & _rhoEA;

  /// rho*V of the junction
  const VariableValue & _rhoV;
  /// rho*u*V of the junction
  const VariableValue & _rhouV;
  /// rho*v*V of the junction
  const VariableValue & _rhovV;
  /// rho*w*V of the junction
  const VariableValue & _rhowV;
  /// rho*E*V of the junction
  const VariableValue & _rhoEV;

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

  /// Single-phase fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Vector of numerical flux user objects for each connected flow channel
  std::vector<const NumericalFlux3EqnBase *> _numerical_flux_uo;
};
