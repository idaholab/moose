/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef DERIVATIVETWOPHASEMATERIAL_H
#define DERIVATIVETWOPHASEMATERIAL_H

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations
class DerivativeTwoPhaseMaterial;

template <>
InputParameters validParams<DerivativeTwoPhaseMaterial>();

/**
 * DerivativeMaterial child class to evaluate a parsed function for the
 * free energy and automatically provide all derivatives.
 * This requires the autodiff patch (https://github.com/libMesh/libmesh/pull/238)
 * to Function Parser in libmesh.
 */
class DerivativeTwoPhaseMaterial : public DerivativeFunctionMaterialBase
{
public:
  DerivativeTwoPhaseMaterial(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeF();
  virtual Real computeDF(unsigned int i_var);
  virtual Real computeD2F(unsigned int i_var, unsigned int j_var);
  virtual Real computeD3F(unsigned int i_var, unsigned int j_var, unsigned int k_var);

  /// Phase parameter (0=A-phase, 1=B-phase)
  const VariableValue & _eta;

  /// name of the order parameter variable
  VariableName _eta_name;

  /// libMesh variable number for eta
  unsigned int _eta_var;

  ///@{
  /// h(eta) switching function
  const MaterialProperty<Real> & _h;
  const MaterialProperty<Real> & _dh;
  const MaterialProperty<Real> & _d2h;
  const MaterialProperty<Real> & _d3h;
  ///@}

  ///@{
  /// g(eta) switching function
  const MaterialProperty<Real> & _g;
  const MaterialProperty<Real> & _dg;
  const MaterialProperty<Real> & _d2g;
  const MaterialProperty<Real> & _d3g;
  ///@}

  /// Phase transformatuion energy barrier
  Real _W;

  /// Function value of the A and B phase.
  const MaterialProperty<Real> &_prop_Fa, &_prop_Fb;

  /// Derivatives of Fa and Fb with respect to arg[i]
  std::vector<const MaterialProperty<Real> *> _prop_dFa, _prop_dFb;

  /// Second derivatives of Fa and Fb.
  std::vector<std::vector<const MaterialProperty<Real> *>> _prop_d2Fa, _prop_d2Fb;

  /// Third derivatives of Fa and Fb.
  std::vector<std::vector<std::vector<const MaterialProperty<Real> *>>> _prop_d3Fa, _prop_d3Fb;
};

#endif // DERIVATIVETWOPHASEMATERIAL_H
