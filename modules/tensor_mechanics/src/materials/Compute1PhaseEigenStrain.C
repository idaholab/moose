#include "Compute1PhaseEigenStrain.h"

template<>
InputParameters validParams<Compute1PhaseEigenStrain>()
{
  InputParameters params = validParams<ComputeEigenStrainBase>();
  params.addClassDescription("Computes an Eigenstrain and its derivatives that is a function of one variable, where the scaler dependence is defined in a derivative material.");
  params.addRequiredParam< std::vector<Real> >("eigen_base","Vector of values defining the constant base tensor for the Eigenstrain, should be length six (e11 e22 e33 e23 e13 e12) or length 9 (column major)");
  params.addRequiredCoupledVar("v","phase variable");
  return params;
}

Compute1PhaseEigenStrain::Compute1PhaseEigenStrain(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<ComputeEigenStrainBase>(name, parameters),
    _v(coupledValue("v")),
    _v_name(getVar("v",0)->name()),
    _var_dep(getMaterialProperty<Real>("var_dep")),
    _dvar_dep_dv(getMaterialPropertyDerivative<Real>("var_dep",_v_name)),
    _d2var_dep_dv2(getMaterialPropertyDerivative<Real>("var_dep",_v_name,_v_name)),
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
Compute1PhaseEigenStrain::computeQpEigenStrain()
{
  //Define Eigenstrain
  _eigen_strain[_qp] = _eigen_base_tensor*_var_dep[_qp];

  //Define derivatives of the elastic strain
  _delastic_strain_dv[_qp] = _eigen_base_tensor*_dvar_dep_dv[_qp];
  _d2elastic_strain_dv2[_qp] = _eigen_base_tensor*_d2var_dep_dv2[_qp];
}

