//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class BarrierFunctionMaterial : public OrderParameterFunctionMaterial
{
public:
  static InputParameters validParams();

  BarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Polynomial order of the switching function \f$ g(\eta) \f$
  MooseEnum _g_order;

  /// zero out g contribution in the eta interval [0:1]
  bool _well_only;
};
