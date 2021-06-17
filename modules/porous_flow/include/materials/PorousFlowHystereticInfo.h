//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowHystereticCapillaryPressure.h"

/**
 * Material designed to calculate the capillary pressure as a function of saturation, or the
 * saturation as a function of capillary pressure, or derivative information, etc
 */
class PorousFlowHystereticInfo : public PorousFlowHystereticCapillaryPressure
{
public:
  static InputParameters validParams();

  PorousFlowHystereticInfo(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Fill _info with the required information
  void computeQpInfo();

  /// Computed nodal or quadpoint value of capillary pressure (needed for hysteretic order computation)
  MaterialProperty<Real> & _pc;

  /// Computed nodal or quadpoint value: the meaning of this depends on info_enum
  MaterialProperty<Real> & _info;

  /// Nodal or quadpoint value of capillary pressure
  const VariableValue & _pc_val;

  /// Nodal or quadpoint value of liquid saturation
  const VariableValue & _sat_val;

  /// small parameter to use in the finite-difference approximations to the derivative
  const Real _fd_eps;

  /// Type of info required
  const enum class InfoTypeEnum {
    PC,
    SAT,
    SAT_GIVEN_PC,
    DS_DPC_ERR,
    DPC_DS_ERR,
    D2S_DPC2_ERR,
    D2PC_DS2_ERR
  } _info_enum;

private:
  /**
   * Computes the relative error between a finite-difference approximation to the derivative and a
   * hand-coded version
   * @return finite_difference / hand_coded - 1, with appropriate guards against division by zero
   * @param finite_difference the finite-difference approximation to the derivative
   * @param hand_coded the hand-coded derivative
   */
  Real relativeError(Real finite_difference, Real hand_coded);
};
