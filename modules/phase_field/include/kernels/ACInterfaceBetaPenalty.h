// Considers cleavage plane anisotropy in the crack propagation

#pragma once

#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "Kernel.h"

class ACInterfaceBetaPenalty;

template <>
InputParameters validParams<ACInterfaceBetaPenalty>();

class ACInterfaceBetaPenalty : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  ACInterfaceBetaPenalty(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  RealGradient gradL();
  RealGradient gradKappa();

  /// the \f$ \nabla(L\psi) \f$ term
  RealGradient nablaLPsi();

  /// the \f$ \kappa\nabla(L\psi) \f$ term
  RealGradient kappaNablaLPsi();

  /// the Mcoupling term
  Real betaNablaPsi();

  /// Mobility
  const MaterialProperty<Real> & _L;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;

  /// flag set if L is a function of non-linear variables in args
  const bool _variable_L;

  /// @{ Mobility derivatives w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;
  const MaterialProperty<Real> & _d2Ldop2;
  /// @}

  /// kappa derivative w.r.t. order parameter
  const MaterialProperty<Real> & _dkappadop;

  /// number of coupled variables
  const unsigned int _nvar;

  /// @{ Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _d2Ldargdop;
  std::vector<std::vector<const MaterialProperty<Real> *>> _d2Ldarg2;
  /// @}

  /// kappa derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dkappadarg;

  /// Gradients for all coupled variables
  std::vector<const VariableGradient *> _gradarg;

private:
  // penalty for damage on planes not normal to the favoured cleavage plane
  // normal (Clayton & Knap, 2015)
  const Real _beta_penalty;

  // normal to the favoured cleavage plane: M in (Clayton & Knap, 2015)
  const std::vector<Real> _cleavage_plane_normal;
};
