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

  RealGradient gradL();
  RealGradient gradKappa();

  /// the \f$ \nabla(L\kappa\psi) \f$ term
  RealGradient nablaLKappaPsi();

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;

  /// flag set if L is a function of non-linear variables in args
  bool _variable_L;

  /// flag set if kappa is a function of non-linear variables in args
  bool _variable_kappa;

  /// @{ Mobility derivatives w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;
  const MaterialProperty<Real> & _d2Ldop2;
  /// @}

  /// @{ kappa derivatives w.r.t. order parameter
  const MaterialProperty<Real> & _dkappadop;
  const MaterialProperty<Real> & _d2kappadop2;
  /// @}

  /// number of coupled variables
  unsigned int _nvar;

  /// @{ Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _d2Ldargdop;
  std::vector<std::vector<const MaterialProperty<Real> *> > _d2Ldarg2;
  /// @}

  /// @{ kappa derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dkappadarg;
  std::vector<const MaterialProperty<Real> *> _d2kappadargdop;
  std::vector<std::vector<const MaterialProperty<Real> *> > _d2kappadarg2;
  /// @}

  /// Gradients for all coupled variables
  std::vector<VariableGradient *> _gradarg;
};

#endif //ACInterface_H
