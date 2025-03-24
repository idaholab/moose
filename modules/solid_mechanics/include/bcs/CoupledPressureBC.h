//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PressureBase.h"

/**
 * Pressure boundary condition using coupled variable to apply pressure in a given direction
 */
template <bool is_ad>
class CoupledPressureBCTempl : public PressureParent<is_ad>
{
public:
  static InputParameters validParams();

  CoupledPressureBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computePressure() const override;

  /// The values of pressure to be imposed
  const GenericVariableValue<is_ad> & _pressure;

  using PressureParent<is_ad>::_qp;
};

typedef CoupledPressureBCTempl<false> CoupledPressureBC;
typedef CoupledPressureBCTempl<true> ADCoupledPressureBC;
