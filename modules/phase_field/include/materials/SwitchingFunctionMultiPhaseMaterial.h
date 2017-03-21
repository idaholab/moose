/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SWITCHINGFUNCTIONMULTIPHASEMATERIAL_H
#define SWITCHINGFUNCTIONMULTIPHASEMATERIAL_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class SwitchingFunctionMultiPhaseMaterial;

template <>
InputParameters validParams<SwitchingFunctionMultiPhaseMaterial>();

/**
 * SwitchingFunctionMultiPhaseMaterial is a switching function for a multi-phase,
 * multi-order parameter system. Defined by Moelans, Acta Mat., v 59, p.1077-1086 (2011).
 * For phase alpha, the switching function is
 * \f$ h_\alpha = (sum_i \eta_{\alpha i}^2) / (sum_\rho sum_i \eta_{\rho i}^2) \f$
 * for phase alpha, where \f$ i \f$  indexes grains of a phase and \f$ rho \f$  indexes phases
 */
class SwitchingFunctionMultiPhaseMaterial : public DerivativeMaterialInterface<Material>
{
public:
  SwitchingFunctionMultiPhaseMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Name of the function
  MaterialPropertyName _h_name;

  /// Order parameters for phase alpha
  unsigned int _num_eta_p;
  std::vector<const VariableValue *> _eta_p;
  std::vector<VariableName> _eta_p_names;

  /// Order parameters for all phases (including alpha)
  unsigned int _num_eta;
  std::vector<const VariableValue *> _eta;
  std::vector<VariableName> _eta_names;

  /// List of which order parameters in the full list of all etas belong to phase p
  std::vector<bool> _is_p;

  /// Switching function and derivatives
  MaterialProperty<Real> & _prop_h;
  std::vector<MaterialProperty<Real> *> _prop_dh;
  std::vector<std::vector<MaterialProperty<Real> *>> _prop_d2h;
};

#endif // SWITCHINGFUNCTIONMULTIPHASEMATERIAL_H
