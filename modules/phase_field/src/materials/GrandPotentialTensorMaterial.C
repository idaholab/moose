//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrandPotentialTensorMaterial.h"

// libMesh includes
#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", GrandPotentialTensorMaterial);

InputParameters
GrandPotentialTensorMaterial::validParams()
{
  InputParameters params = PolycrystalDiffusivityTensorBase::validParams();
  params.addClassDescription("Diffusion and mobility parameters for grand potential model "
                             "governing equations. Uses a tensor diffusivity");
  params.addRequiredParam<Real>("int_width",
                                "The interfacial width in the lengthscale of the problem");
  params.addParam<MaterialPropertyName>("chi", "Coefficient to multiply by D");
  params.addParam<Real>("GBmob0", 0.0, "Grain boundary mobility prefactor");
  params.addRequiredParam<Real>("Q", "Grain boundary migration activation energy in eV");
  params.addParam<Real>(
      "GBMobility", -1, "GB mobility input that overrides the temperature dependent calculation");
  params.addParam<std::string>("f_name", "chiD", "Name for the mobility material property");
  params.addRequiredParam<MaterialPropertyName>("surface_energy", "Surface energy of material");
  params.addParam<std::string>("solid_mobility", "L", "Name of grain mobility for solid phase");
  params.addParam<std::string>("void_mobility", "Lv", "Name of void phase mobility");
  return params;
}

GrandPotentialTensorMaterial::GrandPotentialTensorMaterial(const InputParameters & parameters)
  : PolycrystalDiffusivityTensorBase(parameters),
    _chiD_name(getParam<std::string>("f_name")),
    _chiD(declareProperty<RealTensorValue>(_chiD_name)),
    _dchiDdc(isCoupledConstant(_c_name)
                 ? nullptr
                 : &declarePropertyDerivative<RealTensorValue>(_chiD_name, _c_name)),
    _Ls_name(getParam<std::string>("solid_mobility")),
    _Ls(declareProperty<Real>(_Ls_name)),
    _Lv_name(getParam<std::string>("void_mobility")),
    _Lv(declareProperty<Real>(_Lv_name)),
    _chiDmag(declareProperty<Real>(_chiD_name + "_mag")),
    _sigma_s(getMaterialProperty<Real>("surface_energy")),
    _int_width(getParam<Real>("int_width")),
    _chi_name(getParam<MaterialPropertyName>("chi")),
    _chi(getMaterialProperty<Real>(_chi_name)),
    _dchidc(getMaterialPropertyDerivative<Real>(_chi_name, _c_name)),
    _dchideta(_op_num),
    _dchiDdeta(_op_num),
    _GBMobility(getParam<Real>("GBMobility")),
    _GBmob0(getParam<Real>("GBmob0")),
    _Q(getParam<Real>("Q")),
    _vals_name(_op_num)
{
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals_name[i] = coupledName("v", i);
    _dchideta[i] = &getMaterialPropertyDerivative<Real>(_chi_name, _vals_name[i]);
    if (!isCoupledConstant(_vals_name[i]))
      _dchiDdeta[i] = &declarePropertyDerivative<RealTensorValue>(_chiD_name, _vals_name[i]);
  }
}

void
GrandPotentialTensorMaterial::computeProperties()
{
  PolycrystalDiffusivityTensorBase::computeProperties();

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _chiD[_qp] = _D[_qp] * _chi[_qp];
    if (_dchiDdc)
      (*_dchiDdc)[_qp] = (*_dDdc)[_qp] * _chi[_qp] + _D[_qp] * _dchidc[_qp];
    for (unsigned int i = 0; i < _op_num; ++i)
      if (_dchiDdeta[i])
        (*_dchiDdeta[i])[_qp] = _D[_qp] * (*_dchideta[i])[_qp];

    _chiDmag[_qp] = _chiD[_qp].norm();

    Real GBmob;
    if (_GBMobility < 0)
      GBmob = _GBmob0 * std::exp(-_Q / (_kb * _T[_qp]));
    else
      GBmob = _GBMobility;

    _Ls[_qp] = 4.0 / 3.0 * GBmob / _int_width;
    _Lv[_qp] = 40 * _Ls[_qp];
  }
}
