/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CROSSTERMBARRIERFUNCTION_H
#define CROSSTERMBARRIERFUNCTION_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class CrossTermBarrierFunctionMaterial;

template<>
InputParameters validParams<CrossTermBarrierFunctionMaterial>();

/**
 * MultiBarrierFunctionMaterial is a constraint kernel that acts on one of the eta_i variables to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class CrossTermBarrierFunctionMaterial : public DerivativeMaterialInterface<Material>
{
public:
  CrossTermBarrierFunctionMaterial(const std::string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();

  /// name of the function of eta (used to generate the material property names)
  std::string _function_name;

  /// Polynomial order of the switching function \f$ h(\eta) \f$
  MooseEnum _g_order;

  ///barrier function height vector
  std::vector<Real> _W_ij;

  /// order parameters
  unsigned int _num_eta;
  std::vector<VariableValue *> _eta;

  /// Switching functions and their drivatives
  MaterialProperty<Real> & _prop_g;
  std::vector<MaterialProperty<Real> *> _prop_dg, _prop_d2g;
};

#endif //CROSSTERMBARRIERFUNCTION_H
