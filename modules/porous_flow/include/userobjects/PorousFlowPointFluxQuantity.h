//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Records the instantaneous flux at each Dirac point of a PorousFlow line sink (such as
 * PorousFlowPeacemanBorehole or PorousFlowPolyLineSink).  This is used, for instance, to report
 * the flow-rate profile along a wellbore.  Like PorousFlowSumQuantity, this is a suboptimal
 * setup because it requires a const_cast of a PorousFlowPointFluxQuantity object in order to do
 * the recording.
 */
class PorousFlowPointFluxQuantity : public GeneralUserObject
{
public:
  static InputParameters validParams();

  PorousFlowPointFluxQuantity(const InputParameters & parameters);

  /**
   * Resets the flux recorded at every point to zero, and records the current point
   * coordinates.  The coordinates are (re-)recorded here, rather than once at initialSetup,
   * because the number and position of points of the associated line sink can change during a
   * simulation (eg mesh adaptivity when using line_base, or reporter-defined lines).
   */
  void
  zero(const std::vector<Real> & xs, const std::vector<Real> & ys, const std::vector<Real> & zs);

  /**
   * Adds contrib to the flux recorded at point i
   * @param i the Dirac point ID (as returned by DiracKernel::currentPointCachedID())
   * @param contrib the amount to add to the flux at point i
   */
  void add(std::size_t i, Real contrib);

  /// Does nothing
  virtual void initialize() override;

  /// Does nothing
  virtual void execute() override;

  /// Sums the per-point fluxes over all processors
  virtual void finalize() override;

  /// Returns the flux recorded at each point
  const std::vector<Real> & getFluxes() const { return _fluxes; }

  ///@{ Returns the coordinates of each point
  const std::vector<Real> & getX() const { return _xs; }
  const std::vector<Real> & getY() const { return _ys; }
  const std::vector<Real> & getZ() const { return _zs; }
  ///@}

protected:
  /// The flux at each point of the line sink, indexed by Dirac point ID
  std::vector<Real> _fluxes;

  ///@{ Coordinates of each point of the line sink, indexed by Dirac point ID
  std::vector<Real> _xs;
  std::vector<Real> _ys;
  std::vector<Real> _zs;
  ///@}
};
