//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAnisotropicReturnPlasticityStressUpdateBase.h"

InputParameters
ADAnisotropicReturnPlasticityStressUpdateBase::validParams()
{
  InputParameters params = ADGeneralizedRadialReturnStressUpdate::validParams();

  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  params.set<std::string>("inelastic_strain_rate_name") = "creep_strain_rate";

  return params;
}

ADAnisotropicReturnPlasticityStressUpdateBase::ADAnisotropicReturnPlasticityStressUpdateBase(
    const InputParameters & parameters)
  : ADGeneralizedRadialReturnStressUpdate(parameters),
    _plasticity_strain(declareADProperty<RankTwoTensor>(_base_name + "plastic_strain")),
    _plasticity_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "plastic_strain"))
{
}

void
ADAnisotropicReturnPlasticityStressUpdateBase::initQpStatefulProperties()
{
  _plasticity_strain[_qp].zero();

  ADGeneralizedRadialReturnStressUpdate::initQpStatefulProperties();
}

void
ADAnisotropicReturnPlasticityStressUpdateBase::propagateQpStatefulProperties()
{
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

void
ADAnisotropicReturnPlasticityStressUpdateBase::computeStrainFinalize(
    ADRankTwoTensor & inelasticStrainIncrement,
    const ADRankTwoTensor & /*stress*/,
    const ADDenseVector & /*stress_dev*/,
    const ADReal & /*delta_gamma*/)
{
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp] + inelasticStrainIncrement;
}
