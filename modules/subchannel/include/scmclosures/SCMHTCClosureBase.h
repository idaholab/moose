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

/// structure that holds the needed data to calculate intermediate data needed to calculate the Nusselt number.
struct NusseltPreInfo
{
  Real Re;                 // Reynolds number
  Real Pr;                 // Prandtl number
  Real poD;                // Pitch over diameter ratio
  Real ReL;                // Laminar Reynolds number limit
  Real ReT;                // Turbulent Reynolds number limit
  Real laminar_Nu;         // Laminar Nusselt number
  EChannelType subch_type; // Subchannel type (corner, edge, center)
};

/**
 * Base class for the convective heat transfer coefficients (HTC) closures used in SCM
 */
class SCMHTCClosureBase : public SCMClosureBase
{
public:
  static InputParameters validParams();

  SCMHTCClosureBase(const InputParameters & parameters);

  typedef SubChannel1PhaseProblem::FrictionStruct FrictionStruct;
  typedef SubChannel1PhaseProblem::NusseltStruct NusseltStruct;

  /// @brief Computes the nusselt number for the local conditions
  /// @param friction_info geometrical information about the cell in the channel
  /// @param nusselt_info  flow/coolant information about the cell in the channel
  /// @return the Nusselt Number
  virtual Real computeNusseltNumber(const FrictionStruct & friction_info,
                                    const NusseltStruct & nusselt_info) const = 0;

  /// Computes all the data needed before computing the nusselt number. It's used by all closure models.
  NusseltPreInfo computeNusseltNumberPreInfo(const NusseltStruct & nusselt_info) const;

  /// @brief Computes the convective heat transfer coefficient for the local conditions
  /// @param friction_info geometrical information about the cell in the channel
  /// @param nusselt_info  flow/coolant information about the cell in the channel
  /// @return the convective heat transfer coefficient(HTC)
  Real computeHTC(const FrictionStruct & friction_info,
                  const NusseltStruct & nusselt_info,
                  const Real conduction_k) const;

  SolutionHandle _Dpin_soln;
};
