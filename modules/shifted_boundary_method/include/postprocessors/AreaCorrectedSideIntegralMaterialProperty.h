//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralMaterialProperty.h"
#include "MaterialPropertyInterface.h"

#include "IndexableProperty.h"

#include "BoundaryShortestDistanceToSurface.h"

/**
 * Computes the integral of a material property over a side set.
 */
template <bool is_ad>
class AreaCorrectedSideIntegralMaterialPropertyTempl
  : public SideIntegralMaterialPropertyTempl<is_ad>
{
public:
  static InputParameters validParams();

  AreaCorrectedSideIntegralMaterialPropertyTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  virtual void initialSetup() override final;

  /// Distance and normal provider userobject
  const BoundaryShortestDistanceToSurface * _sbm_distance_uo = nullptr;

  const bool _shifted;
};

typedef AreaCorrectedSideIntegralMaterialPropertyTempl<false>
    AreaCorrectedSideIntegralMaterialProperty;
typedef AreaCorrectedSideIntegralMaterialPropertyTempl<true>
    ADAreaCorrectedSideIntegralMaterialProperty;
