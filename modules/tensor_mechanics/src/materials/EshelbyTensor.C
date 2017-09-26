/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EshelbyTensor.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<EshelbyTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Stuff");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of eigenstrains to be applied in this strain calculation");
  return params;
}

EshelbyTensor::EshelbyTensor(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _sed(declareProperty<Real>(_base_name + "strain_energy_density")),
    _sed_old(getMaterialPropertyOld<Real>(_base_name + "strain_energy_density")),
    _eshelby_tensor(declareProperty<RankTwoTensor>(_base_name + "Eshelby_tensor")),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _grad_disp(3),
    _J_thermal_term_vec(declareProperty<RealVectorValue>("J_thermal_term_vec")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _deigenstrain_dT(_eigenstrain_names.size()),
    _has_temp(isCoupled("temperature")),
    _grad_temp(coupledGradient("temperature"))
{
  unsigned int ndisp = coupledComponents("displacements");

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (ndisp != _mesh.dimension())
    mooseError(
        "The number of variables supplied in 'displacements' must match the mesh dimension.");

  // fetch coupled gradients
  for (unsigned int i = 0; i < ndisp; ++i)
    _grad_disp[i] = &coupledGradient("displacements", i);

  // set unused dimensions to zero
  for (unsigned i = ndisp; i < 3; ++i)
    _grad_disp[i] = &_grad_zero;

  // Get the materials containing the derivatives of the eigenstrains wrt temperature
  if (_has_temp)
  {
    VariableName temp_name = getVar("temperature", 0)->name();
    if (_eigenstrain_names.size() == 0)
      mooseWarning(
          "No 'eigenstrain_names' specified for EshelbyTensor when 'temperature' is specified");

    for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
      _deigenstrain_dT[i] = &getMaterialPropertyDerivative<RankTwoTensor>(
          _base_name + _eigenstrain_names[i], temp_name);
  }
}

void
EshelbyTensor::initQpStatefulProperties()
{
  _sed[_qp] = 0.0;
}

void
EshelbyTensor::computeQpProperties()
{
  _sed[_qp] = _sed_old[_qp] + _stress[_qp].doubleContraction(_strain_increment[_qp]) / 2.0 +
              _stress_old[_qp].doubleContraction(_strain_increment[_qp]) / 2.0;

  RankTwoTensor F((*_grad_disp[0])[_qp],
                  (*_grad_disp[1])[_qp],
                  (*_grad_disp[2])[_qp]); // Deformation gradient
  F.addIa(1.0);
  Real detF = F.det();
  RankTwoTensor FinvT(F.inverse().transpose());

  // 1st Piola-Kirchoff Stress (P):
  RankTwoTensor P = detF * _stress[_qp] * FinvT;

  // FTP = F^T * P = F^T * detF * sigma * FinvT;
  RankTwoTensor FTP = F.transpose() * P;

  RankTwoTensor WI = RankTwoTensor(RankTwoTensor::initIdentity);
  WI *= (_sed[_qp] * detF);

  _eshelby_tensor[_qp] = WI - FTP;

  if (_has_temp && _deigenstrain_dT.size() > 0)
  {
    Real sigma_alpha = 0.0;

    RankTwoTensor total_deigenstrain_dT((*_deigenstrain_dT[0])[_qp]);
    for (unsigned int i = 1; i < _deigenstrain_dT.size(); ++i)
      total_deigenstrain_dT += (*_deigenstrain_dT[i])[_qp];

    sigma_alpha += _stress[_qp].doubleContraction(total_deigenstrain_dT);

    _J_thermal_term_vec[_qp] = sigma_alpha * _grad_temp[_qp];
  }
  else
    _J_thermal_term_vec[_qp].zero();
}
