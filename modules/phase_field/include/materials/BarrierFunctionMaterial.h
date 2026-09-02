//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OrderParameterFunctionMaterial.h"

// Forward Declarations

/**
 * Material class to provide the double well function \f$ g(\eta) \f$ for
 * the KKS system.
 *
 * \see KKSPhaseChemicalPotential
 * \see KKSCHBulk
 */
template <bool is_ad>
class BarrierFunctionMaterialTempl : public OrderParameterFunctionMaterialTempl<is_ad>
{
public:
  static InputParameters validParams();

  BarrierFunctionMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Polynomial order of the switching function \f$ g(\eta) \f$
  MooseEnum _g_order;

  /// zero out g contribution in the eta interval [0:1]
  bool _well_only;
};

typedef BarrierFunctionMaterialTempl<false> BarrierFunctionMaterial;
typedef BarrierFunctionMaterialTempl<true> ADBarrierFunctionMaterial;