#pragma once

#include "VariationalKernelBase.h"
#include "MooseAST.h"
#include "MooseExpressionBuilder.h"

class CahnHilliardVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  CahnHilliardVariational(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual void computeResidual() override;

  virtual void computeJacobian() override;

  virtual void computeOffDiagJacobian(unsigned int jvar) override;

private:
  void buildEnergyFunctional();

  void setupSplitFormulation();

  Real computeDoubleWellDerivative();

  RealVectorValue computeGradientCoefficient();

  const Real _well_height;

  const Real _kappa;

  const Real _mobility;

  const bool _split_formulation;

  const VariableValue * _mu;
  const VariableGradient * _grad_mu;

  unsigned int _mu_var;

  moose::automatic_weak_form::NodePtr _bulk_energy;
  moose::automatic_weak_form::NodePtr _gradient_energy;
  moose::automatic_weak_form::NodePtr _total_energy;

  moose::automatic_weak_form::Differential _variational_derivative;

  bool _use_analytical_jacobian;
};

class FourthOrderCahnHilliardVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  FourthOrderCahnHilliardVariational(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

private:
  void setupHigherOrderSplitting();

  void addSplitVariableKernels();

  const Real _lambda;

  const Real _gamma;

  const bool _enable_sixth_order;

  const Real _beta;

  const VariableValue * _grad_c_x;
  const VariableValue * _grad_c_y;
  const VariableValue * _grad_c_z;

  const VariableGradient * _grad_grad_c_x;
  const VariableGradient * _grad_grad_c_y;
  const VariableGradient * _grad_grad_c_z;

  std::vector<unsigned int> _split_var_nums;

  moose::automatic_weak_form::NodePtr _fourth_order_term;
  moose::automatic_weak_form::NodePtr _sixth_order_term;
};

class AllenCahnVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  AllenCahnVariational(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

private:
  const Real _L;

  const Real _kappa;

  const MaterialProperty<Real> & _f_derivative;

  const MaterialProperty<Real> & _f_second_derivative;

  const bool _use_material_properties;
};

class ElasticVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  ElasticVariational(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  void buildElasticEnergy();

  RankTwoTensor computeStrain();

  RankTwoTensor computeStress(const RankTwoTensor & strain);

  RankFourTensor computeElasticityTensor();

  const unsigned int _component;

  const unsigned int _ndisp;

  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;

  std::vector<unsigned int> _disp_var;

  const Real _lambda;
  const Real _mu;

  const bool _large_deformation;

  moose::automatic_weak_form::NodePtr _elastic_energy;
};

class NeoHookeanVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  NeoHookeanVariational(const InputParameters & parameters);

protected:
  virtual void computeResidual() override;

  virtual void computeJacobian() override;

private:
  RankTwoTensor computeDeformationGradient();

  RankTwoTensor computeRightCauchyGreen(const RankTwoTensor & F);

  RankTwoTensor computeFirstPiolaKirchhoff(const RankTwoTensor & F);

  RankFourTensor computeMaterialTangent(const RankTwoTensor & F);

  const unsigned int _component;

  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;

  const Real _bulk_modulus;
  const Real _shear_modulus;

  const bool _incompressible;

  const VariableValue * _pressure;
  unsigned int _pressure_var;
};

class SurfaceEnergyVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  SurfaceEnergyVariational(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

private:
  RealVectorValue computeInterfaceNormal();

  Real computeAnisotropicCoefficient(const RealVectorValue & normal);

  RealVectorValue computeAnisotropicDerivative(const RealVectorValue & normal);

  const Real _gamma_0;

  const Real _anisotropy_strength;

  const unsigned int _anisotropy_mode;

  const bool _regularize_interface;

  const Real _epsilon_regularization;

  const MaterialProperty<Real> * _gamma_anisotropic;
};

class PhaseFieldCrystalVariational : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  PhaseFieldCrystalVariational(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;

private:
  void setupPFCEnergy();

  const Real _epsilon;

  const Real _q0;

  const Real _temperature;

  const unsigned int _num_modes;

  std::vector<Real> _mode_amplitudes;
  std::vector<RealVectorValue> _mode_vectors;

  const bool _conserved_dynamics;

  moose::automatic_weak_form::NodePtr _pfc_energy;
};

class CoupledCahnHilliardMechanics : public moose::automatic_weak_form::VariationalKernelBase
{
public:
  static InputParameters validParams();

  CoupledCahnHilliardMechanics(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;

  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  void buildCoupledEnergy();

  Real computeEigenstrain(Real concentration);

  RankTwoTensor computeChemoMechanicalStress();

  const bool _solve_displacement;

  const bool _solve_concentration;

  const Real _coupling_coefficient;

  const Real _eigenstrain_coefficient;

  const VariableValue & _c;
  const VariableGradient & _grad_c;
  unsigned int _c_var;

  std::vector<const VariableValue *> _disp;
  std::vector<const VariableGradient *> _grad_disp;
  std::vector<unsigned int> _disp_var;

  moose::automatic_weak_form::NodePtr _coupled_energy;
};
