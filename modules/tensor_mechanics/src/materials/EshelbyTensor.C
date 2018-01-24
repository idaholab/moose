//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EshelbyTensor.h"
#include "RankTwoTensor.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<EshelbyTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Computes the Eshelby tensor as a function of "
                             "strain energy density and the first "
                             "Piola-Kirchoff stress");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addCoupledVar("temperature", "Coupled temperature");
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
    _grad_disp(3),
    _J_thermal_term_vec(declareProperty<RealVectorValue>("J_thermal_term_vec")),
    _has_temp(isCoupled("temperature")),
    _grad_temp(coupledGradient("temperature")),
    _total_deigenstrain_dT(hasMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               ? &getMaterialProperty<RankTwoTensor>("total_deigenstrain_dT")
                               : nullptr)
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

  if (_has_temp && !_total_deigenstrain_dT)
    mooseError("EshelbyTensor Error: To include thermal strain term in Fracture integral "
               "calculation, must both couple temperature in DomainIntegral block and compute "
               "total_deigenstrain_dT using ThermalFractureIntegral material model.");

  if (hasMaterialProperty<RankTwoTensor>(_base_name + "strain_increment"))
  {
    _strain_increment = &getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment");
    _mechanical_strain = nullptr;
  }
  else if (hasMaterialProperty<RankTwoTensor>(_base_name + "mechanical_strain"))
  {
    _mechanical_strain = &getMaterialProperty<RankTwoTensor>(_base_name + "mechanical_strain");
    _strain_increment = nullptr;
  }
  else
    mooseError("EshelbyTensor cannot find either mechanical_strain or strain_increment material "
               "properties.");
}

void
EshelbyTensor::initQpStatefulProperties()
{
  _sed[_qp] = 0.0;
}

void
EshelbyTensor::computeQpProperties()
{
  RankTwoTensor F((*_grad_disp[0])[_qp],
                  (*_grad_disp[1])[_qp],
                  (*_grad_disp[2])[_qp]); // Deformation gradient

  RankTwoTensor H(F);
  F.addIa(1.0);
  Real detF = F.det();
  RankTwoTensor FinvT(F.inverse().transpose());

  if (_strain_increment != nullptr)
    _sed[_qp] = _sed_old[_qp] + _stress[_qp].doubleContraction((*_strain_increment)[_qp]) / 2.0 +
                _stress_old[_qp].doubleContraction((*_strain_increment)[_qp]) / 2.0;
  else
    _sed[_qp] = _stress[_qp].doubleContraction((*_mechanical_strain)[_qp]) / 2.0;

  // 1st Piola-Kirchoff Stress (P):
  RankTwoTensor P = detF * _stress[_qp] * FinvT;

  // HTP = H^T * P = H^T * detF * sigma * FinvT;
  RankTwoTensor HTP = H.transpose() * P;

  RankTwoTensor WI = RankTwoTensor(RankTwoTensor::initIdentity);
  WI *= (_sed[_qp] * detF);

  _eshelby_tensor[_qp] = WI - HTP;

  if (_has_temp)
  {
    Real sigma_alpha = _stress[_qp].doubleContraction((*_total_deigenstrain_dT)[_qp]);
    _J_thermal_term_vec[_qp] = sigma_alpha * _grad_temp[_qp];
  }
  else
    _J_thermal_term_vec[_qp].zero();
}
