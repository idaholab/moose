/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Compute1PhaseEigenStrain.h"

template<>
InputParameters validParams<Compute1PhaseEigenStrain>()
{
  InputParameters params = validParams<ComputeStressFreeStrainBase>();
  params.addClassDescription("Computes an Eigenstrain and its derivatives that is a function of one variable, where the scaler dependence is defined in a derivative material.");
  params.addRequiredParam< std::vector<Real> >("eigen_base","Vector of values defining the constant base tensor for the Eigenstrain, should be length six (e11 e22 e33 e23 e13 e12) or length 9 (column major)");
  params.addRequiredCoupledVar("v","phase variable");
  params.addParam<std::string>("variable_dependence","var_dep","Name of material defining the variable dependence");

  return params;
}

Compute1PhaseEigenStrain::Compute1PhaseEigenStrain(const std::string & name,
                                                 InputParameters parameters) :
    ComputeStressFreeStrainBase(name, parameters),
    _v(coupledValue("v")),
    _v_name(getVar("v",0)->name()),
    _var_dep_name(getParam<std::string>("variable_dependence")),
    _var_dep(getMaterialProperty<Real>(_var_dep_name)),
    _dvar_dep_dv(getMaterialPropertyDerivative<Real>(_var_dep_name,_v_name)),
    _d2var_dep_dv2(getMaterialPropertyDerivative<Real>(_var_dep_name,_v_name,_v_name)),
    _delastic_strain_dv(declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _v_name)),
    _d2elastic_strain_dv2(declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _v_name, _v_name)),
    _eigen_base(getParam< std::vector<Real> >("eigen_base"))
{
  unsigned int len = _eigen_base.size();
  if ( (len != 6) && (len != 9) )
    mooseError("The length of unit_tensor should be 6 or 9 in Compute1PhaseEigenStrain");

  _eigen_base_tensor.fillFromInputVector(_eigen_base);
}

void
Compute1PhaseEigenStrain::computeQpStressFreeStrain()
{
  //Define Eigenstrain
  _stress_free_strain[_qp] = _eigen_base_tensor*_var_dep[_qp];

  //Define derivatives of the elastic strain
  _delastic_strain_dv[_qp] = _eigen_base_tensor*_dvar_dep_dv[_qp];
  _d2elastic_strain_dv2[_qp] = _eigen_base_tensor*_d2var_dep_dv2[_qp];
}

