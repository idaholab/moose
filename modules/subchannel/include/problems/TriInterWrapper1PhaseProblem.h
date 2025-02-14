//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterWrapper1PhaseProblem.h"

class TriInterWrapperMesh;
/**
 * Triangular interwrapper solver
 */
class TriInterWrapper1PhaseProblem : public InterWrapper1PhaseProblem
{
public:
  TriInterWrapper1PhaseProblem(const InputParameters & params);

protected:
  /**
   * Computes the axial friction factor for the sodium coolant and
   * for each subchannel.
   * Upgraded Cheng-Todreas Correlation (2018).
   */
  virtual Real computeFrictionFactor(Real Re) override;
  /// computeDP(int iz) is defined/overridden in order to use the friction factor for sodium
  virtual void computeDP(int iblock) override;
  /// computeMassFlowForDPDZ() and enforceUniformDPDZAtInlet()
  /// are overriden to define the sodium friction factor
  virtual void computeh(int iblock) override;
  virtual void externalSolve() override;
  /// average relative error in pressure drop of channels
  Real _dpz_error;
  const TriInterWrapperMesh & _tri_sch_mesh;

public:
  static InputParameters validParams();
};
