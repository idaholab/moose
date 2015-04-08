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

template<>
InputParameters validParams<MultiBarrierFunctionMaterial>();

/**
 * MultiBarrierFunctionMaterial is a constraint kernel that acts on one of the eta_i variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class MultiBarrierFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  MultiBarrierFunctionMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  /// name of the function of eta (used to generate the material property names)
  std::string _function_name;

  /// Polynomial order of the switching function \f$ h(\eta) \f$
  MooseEnum _g_order;

  /// order parameters
  unsigned int _num_eta;
  std::vector<VariableValue *> _eta;

  /// Switching functions and their drivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg, _prop_d2g;
};

#endif //MULTIBARRIERFUNCTION_H
