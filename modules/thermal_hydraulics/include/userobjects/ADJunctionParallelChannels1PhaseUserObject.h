//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVolumeJunction1PhaseUserObject.h"

class SinglePhaseFluidProperties;
class ADNumericalFlux3EqnBase;

/**
 * Computes and caches flux and residual vectors for a 1-phase junction that connects flow channels
 * that are parallel
 *
 * This class computes and caches the following quantities:
 * \li residuals for the scalar variables associated with the junction, and
 * \li fluxes between the flow channels and the junction.
 */
class ADJunctionParallelChannels1PhaseUserObject : public ADVolumeJunction1PhaseUserObject
{
public:
  ADJunctionParallelChannels1PhaseUserObject(const InputParameters & params);

  virtual void initialize() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual void finalize() override;

protected:
  virtual void computeFluxesAndResiduals(const unsigned int & c) override;

  /// Pressure material property
  const ADMaterialProperty<Real> & _p;

  /// Channel direction for the first connection
  RealVectorValue _dir_c0;
  /// Flow direction for the first connection
  RealVectorValue _d_flow;

  //// Stored pressure for each connection
  std::vector<ADReal> _stored_pA;

  /// Areas at each connection
  std::vector<ADReal> _areas;
  /// Check if the connection is an inlet
  std::vector<bool> _is_inlet;

  /// Connection index for inlet flow channel connections
  std::vector<unsigned int> _c_in;
  /// Connection index for outlet flow channel connections
  std::vector<unsigned int> _c_out;
  /// Connection index for connections that contribute to the wall pressure
  std::vector<unsigned int> _c_wall;

  /// Name of the associated component
  const std::string & _component_name;

public:
  static InputParameters validParams();
};
