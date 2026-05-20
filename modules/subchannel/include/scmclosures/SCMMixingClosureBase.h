//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMClosureBase.h"
#include "SubChannel1PhaseProblem.h"

/**
 * Base class for turbulent mixing closures used in SCM
 */
class SCMMixingClosureBase : public SCMClosureBase
{
public:
  static InputParameters validParams();

  SCMMixingClosureBase(const InputParameters & parameters);

  typedef SubChannel1PhaseProblem::FrictionStruct FrictionStruct;

  /// @brief Computes the turbulent mixing coefficient for the local conditions around gap(i_gap) and axial level(iz)
  /// @param i_gap index of the gap
  /// @return the mixing coefficient (beta)
  virtual Real computeMixingParameter(const unsigned int i_gap, const unsigned int iz) const = 0;

  /// @brief Computes the wire-wrap sweep-flow coefficient for peripheral gaps.
  /// @return the sweep-flow coefficient; defaults to zero for closures without sweep flow.
  virtual Real computeSweepFlowMixingParameter(const unsigned int i_gap,
                                               const unsigned int iz) const;

  /// Turbulent modeling parameter used in axial momentum equation
  const Real _CT;

  /**
   * Return the Turbulent modeling parameter
   */
  virtual Real getCT() const { return _CT; }
};
