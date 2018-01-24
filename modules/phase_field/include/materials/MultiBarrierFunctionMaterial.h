/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTIBARRIERFUNCTION_H
#define MULTIBARRIERFUNCTION_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class MultiBarrierFunctionMaterial;

template <>
InputParameters validParams<MultiBarrierFunctionMaterial>();

/**
 * Double well phase transformation barrier free energy contribution.
 *\f$ g(\vec\eta) = \sum_i \eta_i^2(1-\eta_i)^2 \f$
 */
class MultiBarrierFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
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
  unsigned int _num_eta;
  std::vector<const VariableValue *> _eta;

  /// Barrier functions and their drivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg, _prop_d2g;
};

#endif // MULTIBARRIERFUNCTION_H
