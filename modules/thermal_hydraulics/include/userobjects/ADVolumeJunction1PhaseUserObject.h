//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunctionBaseUserObject.h"
#include "SlopeReconstruction1DInterface.h"

class SinglePhaseFluidProperties;
class ADNumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase volume junction
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class ADVolumeJunction1PhaseUserObject : public ADVolumeJunctionBaseUserObject,
                                         public SlopeReconstruction1DInterface<true>
{
public:
  ADVolumeJunction1PhaseUserObject(const InputParameters & params);

  virtual void finalize() override;

  /**
   * Computes the flux between the junction and a flow channel, seen from the junction
   *
   * @param[in] numerical_flux Numerical flux user object of the flow channel
   * @param[in] Ui Flow channel solution at the junction face, in 3-D form
   * @param[in] ni Outward normal of the flow channel at the junction face
   * @param[in] g_dot_dxJ Gravity vector dotted with the displacement from the junction center to
   * the face; zero when the hydrostatic correction is off
   */
  std::vector<ADReal> compute3DFlux(const ADNumericalFlux3EqnBase & numerical_flux,
                                    const std::vector<ADReal> & Ui,
                                    const RealVectorValue & ni,
                                    const Real g_dot_dxJ = 0.0) const;

  /**
   * Computes the junction residual contribution of one flow channel connection
   *
   * @param[in] flux_3d Flux computed by compute3DFlux()
   * @param[in] Ui Flow channel solution at the junction face, in 3-D form
   * @param[in] ni Outward normal of the flow channel at the junction face
   * @param[in] is_primary_connection Is this the first connection?
   * @param[in] g_dot_dxJ Same as in compute3DFlux()
   */
  std::vector<ADReal> computeResidual(const std::vector<ADReal> & flux_3d,
                                      const std::vector<ADReal> & Ui,
                                      const RealVectorValue & ni,
                                      bool is_primary_connection,
                                      const Real g_dot_dxJ = 0.0) const;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;
  virtual std::vector<const MooseVariableBase *> getFlowChannelVariables() const override;
  virtual std::vector<const MooseVariableBase *> getJunctionVariables() const override;
  virtual std::vector<ADReal> computeElementPrimitiveVariables(const Elem * elem) const override;

  /**
   * Computes the flow channel solution at the junction face of the current connection, in 3-D form
   *
   * Without the hydrostatic correction, this is the cell-average solution of the element next to
   * the junction. With the correction, it is the slope reconstruction of that element with the
   * junction as its missing neighbor, placed at the mirror point of the cell center across the
   * face, with the junction solution moved to that point along the hydrostatic balance.
   */
  std::vector<ADReal> computeFlowChannelFaceSolution() const;

  /**
   * Computes the junction solution at a point
   *
   * Density and velocity are those of the junction. With the hydrostatic correction, the pressure
   * is moved from the junction center to the point along the hydrostatic balance, and the
   * specific internal energy follows it at constant density, as in the ghost cell of
   * ADBoundaryFlux3EqnGhostPressure.
   *
   * @param[in] g_dot_dx Gravity vector dotted with the displacement from the junction center to
   * the point
   * @param[out] rhoJ Junction density
   * @param[out] uvecJ Junction velocity vector
   * @param[out] eJ Junction specific internal energy at the point
   * @param[out] pJ Junction pressure at the point
   */
  void computeJunctionSolution(const Real g_dot_dx,
                               ADReal & rhoJ,
                               ADRealVectorValue & uvecJ,
                               ADReal & eJ,
                               ADReal & pJ) const;

  /// Cross-sectional area of connected flow channels, elemental
  const ADVariableValue & _A;
  /// Cross-sectional area of connected flow channels, linear; evaluated at the connection
  const ADVariableValue & _A_linear;
  /// rho*A of the connected flow channels
  const ADVariableValue & _rhoA;
  /// rho*u*A of the connected flow channels
  const ADVariableValue & _rhouA;
  /// rho*E*A of the connected flow channels
  const ADVariableValue & _rhoEA;

  /// Flow channel solution variables, for the slope reconstruction at the junction face
  std::vector<MooseVariable *> _U_vars;

  /// Form loss coefficient
  const Real & _K;
  /// Reference area
  const Real & _A_ref;

  /// Apply velocity scaling?
  const bool _apply_velocity_scaling;

  /// Use the hydrostatic correction at the junction faces?
  const bool _use_hydrostatic_correction;
  /// Gravitational acceleration vector, used by the hydrostatic correction
  const RealVectorValue _gravity_vector;
  /// Spatial position of the center of the junction, used by the hydrostatic correction
  const Point _position;

  /// Single-phase fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Vector of numerical flux user objects for each connected flow channel
  std::vector<const ADNumericalFlux3EqnBase *> _numerical_flux_uo;

public:
  static InputParameters validParams();
};
