//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class TwoPhaseFluidProperties;
class SinglePhaseFluidProperties;

/**
 * Computes the average of the densities of the phases corresponding to a
 * 2-phase fluid properties object.
 *
 * The computed physical quantity does not have any physical significance; this
 * aux kernel is used for testing 2-phase fluid properties classes.
 */
class TwoPhaseAverageDensityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TwoPhaseAverageDensityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Pressure
  const VariableValue & _p;
  /// Temperature
  const VariableValue & _T;

  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
  /// Liquid 1-phase fluid properties object
  const SinglePhaseFluidProperties & _fp_liquid;
  /// Vapor 1-phase fluid properties object
  const SinglePhaseFluidProperties & _fp_vapor;
};
