//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPhaseStressMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("TensorMechanicsApp", MultiPhaseStressMaterial);

InputParameters
MultiPhaseStressMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a global stress form multiple phase stresses");
  params.addParam<std::vector<MaterialPropertyName>>(
      "h", "Switching Function Materials that provide h(eta_i)");
  params.addRequiredParam<std::vector<std::string>>("phase_base",
                                                    "Base names for the Phase strains");
  params.addParam<std::string>("base_name", "Base name for the computed global stress (optional)");
  return params;
}

MultiPhaseStressMaterial::MultiPhaseStressMaterial(const InputParameters & parameters)
  : Material(parameters),
    _h_list(getParam<std::vector<MaterialPropertyName>>("h")),
    _n_phase(_h_list.size()),
    _h_eta(_n_phase),
    _phase_base(getParam<std::vector<std::string>>("phase_base")),
    _phase_stress(_n_phase),
    _dphase_stress_dstrain(_n_phase),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _dstress_dstrain(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult"))
{
  // verify parameter length
  if (_n_phase != _phase_base.size())
    mooseError(
        "h and phase_base input vectors need to have the same length in MultiPhaseStressMaterial ",
        name());

  for (unsigned int i = 0; i < _n_phase; ++i)
  {
    _h_eta[i] = &getMaterialProperty<Real>(_h_list[i]);
    _phase_stress[i] = &getMaterialProperty<RankTwoTensor>(_phase_base[i] + "_stress");
    _dphase_stress_dstrain[i] =
        &getMaterialProperty<RankFourTensor>(_phase_base[i] + "_Jacobian_mult");
  }
}

void
MultiPhaseStressMaterial::computeQpProperties()
{
  _stress[_qp].zero();
  _dstress_dstrain[_qp].zero();

  for (unsigned int i = 0; i < _n_phase; ++i)
  {
    _stress[_qp] += (*_h_eta[i])[_qp] * (*_phase_stress[i])[_qp];
    _dstress_dstrain[_qp] += (*_h_eta[i])[_qp] * (*_dphase_stress_dstrain[i])[_qp];
  }
}
