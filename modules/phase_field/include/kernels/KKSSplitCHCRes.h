//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SplitCHBase.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/**
 * SplitCHBulk child class that takes all the necessary data from a
 * KKSBaseMaterial.
 * We calculate \f$ \frac{\partial F_a}{\partial c_a} \f$.
 * This takes advantage of the KKS identity
 *
 * \f$ dF/dc = dF_a/dc_a (= dF_b/dc_b) \f$
 *
 * The non-linear variable for this Kernel is the concentration 'c'.
 * The user picks one phase free energy \f$ F_a \f$ (f_base) and its corresponding
 * phase concentration \f$ c_a \f$
 */
class KKSSplitCHCRes : public DerivativeMaterialInterface<JvarMapKernelInterface<SplitCHBase>>
{
public:
  static InputParameters validParams();

  KKSSplitCHCRes(const InputParameters & parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeQpResidual();
  virtual void initialSetup();

private:
  ///@{ Phase concentration variable
  unsigned int _ca_var;
  VariableName _ca_name;
  ///@}

  /// chemical potential
  const MaterialProperty<Real> & _dFadca;

  /// Second derivatives of fa with respect to all ca and coupled variables
  std::vector<const MaterialProperty<Real> *> _d2Fadcadarg;

  /// Chemical potential
  unsigned int _w_var;
  const VariableValue & _w;
};
