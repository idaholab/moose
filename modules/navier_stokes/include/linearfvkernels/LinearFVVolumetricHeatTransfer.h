//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"

/**
 * Kernel that adds contributions to the system matrix and right hand side based on
 * heat transfer between a solid and a fluid.
 * $Q = -h_{vol}*(T_{fluid}-T_{solid})$
 * where h is a volumetric heat transfer coefficient that can be determined as:
 * $h_{vol}=h_{surf}*A/V$
 */
class LinearFVVolumetricHeatTransfer : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param params The InputParameters for the kernel.
   */
  LinearFVVolumetricHeatTransfer(const InputParameters & params);

  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;

protected:
  /// Routine used to throw an error if the provided variable is not an
  /// MooseLinearVariableFV
  const MooseLinearVariableFV<Real> & getTemperatureVariable(const std::string & vname);
  /// MOOSE functor describing the heat transfer coefficient
  const Moose::Functor<Real> & _h_solid_fluid;
  /// Reference to the linear finite volume fluid temperature variable
  const MooseLinearVariableFV<Real> & _temp_fluid;
  /// Reference to the linear finite volume solid temperature variable
  const MooseLinearVariableFV<Real> & _temp_solid;
  /// Flag to help the kernel to decide if it is executed on a solid of a fluid
  const bool _is_solid;
};
