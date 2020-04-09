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

/**
 * This kernel adds the term (dFdv, test), where v is a coupled variable.
 */
class CoupledMaterialDerivative : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  CoupledMaterialDerivative(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// name of the coupled variable the derivative is taken with respect to
  std::string _v_name;
  unsigned int _v_var;

  /// Material property derivative w.r.t. v
  const MaterialProperty<Real> & _dFdv;

  /// 2nd order material property derivative w.r.t. v then u
  const MaterialProperty<Real> & _d2Fdvdu;

  /// 2nd order material property derivatives w.r.t. v and then all other coupled variables
  std::vector<const MaterialProperty<Real> *> _d2Fdvdarg;
};
