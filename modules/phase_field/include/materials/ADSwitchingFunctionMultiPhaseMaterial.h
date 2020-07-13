//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"
#include "DerivativeMaterialPropertyNameInterface.h"

// Forward Declarations

/**
 * ADSwitchingFunctionMultiPhaseMaterial is a switching function for a multi-phase,
 * multi-order parameter system. Defined by Moelans, Acta Mat., v 59, p.1077-1086 (2011).
 * For phase alpha, the switching function is
 * \f$ h_\alpha = (sum_i \eta_{\alpha i}^2) / (sum_\rho sum_i \eta_{\rho i}^2) \f$
 * for phase alpha, where \f$ i \f$  indexes grains of a phase and \f$ rho \f$  indexes phases
 */

class ADSwitchingFunctionMultiPhaseMaterial : public ADMaterial,
                                              public DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();

  ADSwitchingFunctionMultiPhaseMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Name of the function
  MaterialPropertyName _h_name;

  /// Order parameters for phase alpha
  unsigned int _num_eta_p;
  std::vector<const ADVariableValue *> _eta_p;
  std::vector<VariableName> _eta_p_names;

  /// Order parameters for all phases (including alpha)
  unsigned int _num_eta;
  std::vector<const ADVariableValue *> _eta;
  std::vector<VariableName> _eta_names;

  /// List of which order parameters in the full list of all etas belong to phase p
  std::vector<bool> _is_p;

  /// Switching function and derivatives
  ADMaterialProperty<Real> & _prop_h;
  std::vector<ADMaterialProperty<Real> *> _prop_dh;
};
