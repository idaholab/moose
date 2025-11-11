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

struct NusseltPreInfo
{
  Real Re;
  Real Pr;
  Real poD;
  Real ReL;
  Real ReT;
  Real laminar_Nu;
  EChannelType subch_type;
};

/**
 * Base class for heat transfer coefficients (HTC) closures used in SCM
 */
class SCMHTCClosureBase : public SCMClosureBase
{
public:
  static InputParameters validParams();

  SCMHTCClosureBase(const InputParameters & parameters);

  typedef SubChannel1PhaseProblem::FrictionStruct FrictionStruct;
  typedef SubChannel1PhaseProblem::NusseltStruct NusseltStruct;

  /// @brief Computes the friction factor for the local conditions
  /// @param friction_info geometrical information about the cell in the channel
  /// @param nusselt_info  flow/coolant information about the cell in the channel
  /// @return the Nusselt Number
  virtual Real computeNusseltNumber(const FrictionStruct & friction_info,
                                    const NusseltStruct & nusselt_info) const = 0;

  virtual NusseltPreInfo
  computeNusseltNumberPreInfo(const NusseltStruct & nusselt_info) const final;

  virtual Real computeHTC(const FrictionStruct & friction_info,
                          const NusseltStruct & nusselt_info,
                          const Real & conduction_k) const = 0;
};
