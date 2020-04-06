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
 * \f$ \left( \kappa \nabla \eta_{\alpha i}, \nabla (L \psi) \right) \f$
 * It is assumed kappa is a function of gradients of two order parameters eta_alpha and eta_beta.
 * Therefore, kappa depends on multiple order parameters.
 */
class ACInterface2DMultiPhase2 : public ACInterface
{
public:
  static InputParameters validParams();

  ACInterface2DMultiPhase2(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Interfacial parameter
  const MaterialProperty<RealGradient> & _dkappadgrad_etaa;
};
