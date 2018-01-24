/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SWITCHINGFUNCTIONPENALTY_H
#define SWITCHINGFUNCTIONPENALTY_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class SwitchingFunctionPenalty;

template <>
InputParameters validParams<SwitchingFunctionPenalty>();

/**
 * SwitchingFunctionPenalty is a constraint kernel adds a penalty
 * to each order parameter to
 * enforce \f$ \sum_n h_i(\eta_i) \equiv 1 \f$.
 */
class SwitchingFunctionPenalty : public DerivativeMaterialInterface<Kernel>
{
public:
  SwitchingFunctionPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Switching function names
  std::vector<MaterialPropertyName> _h_names;
  unsigned int _num_h;

  /// Switching functions and their drivatives
  std::vector<const MaterialProperty<Real> *> _h, _dh;
  const MaterialProperty<Real> * _d2h;

  /// Penalty pre-factor
  const Real _penalty;

  /// number of non-linear variables in the problem
  const unsigned int _number_of_nl_variables;

  /// eta index for the j_vars in the jacobian computation
  std::vector<int> _j_eta;

  /// Index of the eta this kernel is operating on
  int _a;
};

#endif // SWITCHINGFUNCTIONPENALTY_H
