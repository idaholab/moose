/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACInterface_H
#define ACInterface_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterface;

template<>
InputParameters validParams<ACInterface>();

class ACInterface : public DerivativeMaterialInterface<JvarMapInterface<Kernel> >
{
public:
  ACInterface(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;

  /// flag set if L is a function of non-linear variables in args
  bool _variable_L;

  /// flag set if kappa is a function of non-linear variables in args
  bool _variable_kappa;

  /// Mobility derivative w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;

  /// kappa derivative w.r.t. order parameter
  const MaterialProperty<Real> & _dkappadop;

  /// number of coupled variables
  unsigned int _nvar;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;

  /// kappa derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dkappadarg;

  /// Gradients for all coupled variables
  std::vector<VariableGradient *> _gradarg;
};

#endif //ACInterface_H
