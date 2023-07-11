//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeFunctionMaterialBase.h"
#include "ADRankTwoTensorForward.h"
#include "ADRankFourTensorForward.h"

/**
 * Material class to compute the elastic free energy and its derivatives
 */
class ElasticEnergyMaterialSAS : public DerivativeFunctionMaterialBase
{
public:
  static InputParameters validParams();

  ElasticEnergyMaterialSAS(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Real computeF() override;
  virtual Real computeDF(unsigned int i_var) override;

  const MaterialProperty<Real> &_prop_Fa, &_prop_Fb;
  
  const std::string _base_name;
  std::string _base_A;
  std::string _base_B;

  const unsigned int _num_eta;
  const std::vector<const VariableValue *> _eta;
  const std::vector<VariableName> _eta_names;

  /// Switching Function and its derivative
  //const MaterialPropertyName _h_name; 
  std::string _h_name;
  const MaterialProperty<Real> & _h_eta;
  std::vector<const MaterialProperty<Real> *> _dh_eta;

  /// Stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  ///@{ Strain and derivatives
  const MaterialProperty<RankTwoTensor> & _strain_A;
  const MaterialProperty<RankTwoTensor> & _strain_B;

  const MaterialProperty<RankTwoTensor> & _eigenstrain_B;
};
