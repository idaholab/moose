/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSSPLITCHCRES_H
#define KKSSPLITCHCRES_H

#include "SplitCHBase.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class KKSSplitCHCRes;

template <>
InputParameters validParams<KKSSplitCHCRes>();

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
  KKSSplitCHCRes(const InputParameters & parameters);

protected:
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeQpResidual();
  virtual void initialSetup();

private:
  /// Number of coupled variables
  unsigned int _nvar;

  ///@{
  /// Phase concnetration variables
  unsigned int _ca_var;
  VariableName _ca_name;
  unsigned int _cb_var;
  VariableName _cb_name;
  ///@}

  /// Derivatives of \f$ dFa/dca \f$ with respect to all coupled variables
  std::vector<const MaterialProperty<Real> *> _d2Fadcadarg;

  /// h(eta) material property
  const MaterialProperty<Real> & _prop_h;

  /// Second derivative \f$ d^2Fa/dca^2 \f$
  const MaterialProperty<Real> & _first_derivative_Fa;

  /// Second derivative \f$ d^2Fa/dca^2 \f$
  const MaterialProperty<Real> & _second_derivative_Fa;

  /// Second derivative \f$ d^2Fb/dcb^2 \f$
  const MaterialProperty<Real> & _second_derivative_Fb;

  /// Chemical potential
  unsigned int _w_var;
  const VariableValue & _w;
};

#endif // KKSSPLITCHCRES_H
