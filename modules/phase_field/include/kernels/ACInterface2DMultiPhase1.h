//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"
#include "ACInterface.h"

/**
 * Compute the Allen-Cahn interface term with the weak form residual
 * \f$ \left(\nabla (L \psi), 1/2 {\partial \kappa} \over {\partial \nabla \eta_{\alpha i}}
 * \sum \{(\nabla \eta_{\beta j})^2 \} \right) \f$
 */
class ACInterface2DMultiPhase1 : public ACInterface
{
public:
  static InputParameters validParams();

  ACInterface2DMultiPhase1(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real sumSquareGradEta();

  /// Interfacial parameter
  const MaterialProperty<RealGradient> & _dkappadgrad_etaa;
  const MaterialProperty<RealTensorValue> & _d2kappadgrad_etaa;

  /// Order parameters
  unsigned int _num_etas;
  std::vector<const VariableValue *> _eta;
  std::vector<const VariableGradient *> _grad_eta;
};
