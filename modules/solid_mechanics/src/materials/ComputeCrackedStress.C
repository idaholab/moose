//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrackedStress.h"

registerMooseObject("TensorMechanicsApp", ComputeCrackedStress);

InputParameters
ComputeCrackedStress::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes energy and modifies the stress for phase field fracture");
  params.addRequiredCoupledVar("c", "Order parameter for damage");
  params.addParam<Real>("kdamage", 1e-9, "Stiffness of damaged matrix");
  params.addParam<bool>("finite_strain_model", false, "The model is using finite strain");
  params.addParam<bool>(
      "use_current_history_variable", false, "Use the current value of the history variable.");
  params.addParam<MaterialPropertyName>(
      "F_name", "E_el", "Name of material property storing the elastic energy");
  params.addParam<MaterialPropertyName>(
      "kappa_name",
      "kappa_op",
      "Name of material property being created to store the interfacial parameter kappa");
  params.addParam<MaterialPropertyName>(
      "mobility_name", "L", "Name of material property being created to store the mobility L");
  params.addParam<std::string>("base_name", "The base name used to save the cracked stress");
  params.addRequiredParam<std::string>("uncracked_base_name",
                                       "The base name used to calculate the original stress");
  return params;
}

ComputeCrackedStress::ComputeCrackedStress(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _uncracked_base_name(getParam<std::string>("uncracked_base_name") + "_"),
    _finite_strain_model(getParam<bool>("finite_strain_model")),
    _use_current_hist(getParam<bool>("use_current_history_variable")),
    _strain(
        _finite_strain_model
            ? getMaterialPropertyByName<RankTwoTensor>(_uncracked_base_name + "elastic_strain")
            : getMaterialPropertyByName<RankTwoTensor>(_uncracked_base_name + "mechanical_strain")),
    _uncracked_stress(getMaterialPropertyByName<RankTwoTensor>(_uncracked_base_name + "stress")),
    _uncracked_Jacobian_mult(
        getMaterialPropertyByName<RankFourTensor>(_uncracked_base_name + "Jacobian_mult")),
    _c(coupledValue("c")),
    _gc_prop(getMaterialProperty<Real>("gc_prop")),
    _l(getMaterialProperty<Real>("l")),
    _visco(getMaterialProperty<Real>("visco")),
    _kdamage(getParam<Real>("kdamage")),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _F(declareProperty<Real>(getParam<MaterialPropertyName>("F_name"))),
    _dFdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("F_name"),
                                          coupledName("c", 0))),
    _d2Fdc2(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("F_name"), coupledName("c", 0), coupledName("c", 0))),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _dstress_dc(declarePropertyDerivative<RankTwoTensor>("stress", coupledName("c", 0))),
    _hist(declareProperty<Real>("hist")),
    _hist_old(getMaterialPropertyOld<Real>("hist")),
    _Jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _kappa(declareProperty<Real>(getParam<MaterialPropertyName>("kappa_name"))),
    _L(declareProperty<Real>(getParam<MaterialPropertyName>("mobility_name")))
{
}

void
ComputeCrackedStress::initQpStatefulProperties()
{
  _stress[_qp].zero();
  _hist[_qp] = 0.0;
}

void
ComputeCrackedStress::computeQpProperties()
{
  const Real c = _c[_qp];

  // Zero out values when c > 1
  Real cfactor = 1.0;
  if (c > 1.0)
    cfactor = 0.0;

  // Create the positive and negative projection tensors
  RankFourTensor I4sym(RankFourTensor::initIdentitySymmetricFour);
  std::vector<Real> eigval;
  RankTwoTensor eigvec;
  RankFourTensor Ppos = _uncracked_stress[_qp].positiveProjectionEigenDecomposition(eigval, eigvec);
  RankFourTensor Pneg = I4sym - Ppos;

  // Project the positive and negative stresses
  RankTwoTensor stress0pos = Ppos * _uncracked_stress[_qp];
  RankTwoTensor stress0neg = Pneg * _uncracked_stress[_qp];

  // Compute the positive and negative elastic energies
  Real G0_pos = (stress0pos).doubleContraction(_strain[_qp]) / 2.0;
  Real G0_neg = (stress0neg).doubleContraction(_strain[_qp]) / 2.0;

  // Update the history variable
  if (G0_pos > _hist_old[_qp])
    _hist[_qp] = G0_pos;
  else
    _hist[_qp] = _hist_old[_qp];

  Real hist_variable = _hist_old[_qp];
  if (_use_current_hist)
    hist_variable = _hist[_qp];

  // Compute degredation function and derivatives
  Real h = cfactor * (1.0 - c) * (1.0 - c) * (1.0 - _kdamage) + _kdamage;
  Real dhdc = -2.0 * cfactor * (1.0 - c) * (1.0 - _kdamage);
  Real d2hdc2 = 2.0 * cfactor * (1.0 - _kdamage);

  // Compute stress and its derivatives
  _stress[_qp] = (Ppos * h + Pneg) * _uncracked_stress[_qp];
  _dstress_dc[_qp] = stress0pos * dhdc;
  _Jacobian_mult[_qp] = (Ppos * h + Pneg) * _uncracked_Jacobian_mult[_qp];

  // Compute energy and its derivatives
  _F[_qp] = hist_variable * h - G0_neg + _gc_prop[_qp] * c * c / (2 * _l[_qp]);
  _dFdc[_qp] = hist_variable * dhdc + _gc_prop[_qp] * c / _l[_qp];
  _d2Fdc2[_qp] = hist_variable * d2hdc2 + _gc_prop[_qp] / _l[_qp];

  // 2nd derivative wrt c and strain = 0.0 if we used the previous step's history varible
  if (_use_current_hist)
    _d2Fdcdstrain[_qp] = stress0pos * dhdc;

  // Assign L and kappa
  _kappa[_qp] = _gc_prop[_qp] * _l[_qp];
  _L[_qp] = 1.0 / (_gc_prop[_qp] * _visco[_qp]);
}
