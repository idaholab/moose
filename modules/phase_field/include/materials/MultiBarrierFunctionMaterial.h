//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * Double well phase transformation barrier free energy contribution.
 *\f$ g(\vec\eta) = \sum_i \eta_i^2(1-\eta_i)^2 \f$
 */
class MultiBarrierFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  static InputParameters validParams();

  MultiBarrierFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// name of the function of eta (used to generate the material property names)
  std::string _function_name;

  /// Polynomial order of the barrier function \f$ g(\eta) \f$
  MooseEnum _g_order;

  /// zero out g contribution in the eta interval [0:1]
  bool _well_only;

  /// order parameters
  const unsigned int _num_eta;
  const std::vector<const VariableValue *> _eta;

  /// Barrier functions and their drivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg, _prop_d2g;
};
