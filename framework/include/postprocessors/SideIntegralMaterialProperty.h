//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"
#include "MaterialPropertyInterface.h"

#include "IndexableProperty.h"

/**
 * Computes the integral of a material property over a side set.
 */
template <bool is_ad>
class SideIntegralMaterialPropertyTempl : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  SideIntegralMaterialPropertyTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeQpIntegral() override;

  const IndexableProperty<SideIntegralPostprocessor, is_ad> _prop;
};

typedef SideIntegralMaterialPropertyTempl<false> SideIntegralMaterialProperty;
typedef SideIntegralMaterialPropertyTempl<true> ADSideIntegralMaterialProperty;
