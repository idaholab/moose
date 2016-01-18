/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeStressFreeStrainBase.h"

template<>
InputParameters validParams<ComputeStressFreeStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addParam<bool>("incremental_form", false, "Should the StressFreestrain be in incremental form for finite strain methods?");
  return params;
}

ComputeStressFreeStrainBase::ComputeStressFreeStrainBase(const InputParameters & parameters) :
    Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _incremental_form(getParam<bool>("incremental_form")),
    _stress_free_strain(declareProperty<RankTwoTensor>(_base_name + "stress_free_strain")),
    _stress_free_strain_old(_incremental_form ? &declarePropertyOld<RankTwoTensor>(_base_name + "stress_free_strain") : NULL),
    _stress_free_strain_increment(declareProperty<RankTwoTensor>(_base_name + "stress_free_strain_increment"))
{
}

void
ComputeStressFreeStrainBase::initQpStatefulProperties()
{
  _stress_free_strain[_qp].zero();
  if (_incremental_form)
    (*_stress_free_strain_old)[_qp] = _stress_free_strain[_qp];

  _stress_free_strain_increment[_qp].zero();
}

void
ComputeStressFreeStrainBase::computeQpProperties()
{
  computeQpStressFreeStrain();

  if (_incremental_form)
    _stress_free_strain_increment[_qp] = _stress_free_strain[_qp] - (*_stress_free_strain_old)[_qp];
  else
    _stress_free_strain_increment[_qp].zero();
}
