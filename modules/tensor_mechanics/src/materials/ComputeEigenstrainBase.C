/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

template<>
InputParameters validParams<ComputeEigenstrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addParam<bool>("incremental_form", false, "Should the eigenstrain be in incremental form (for incremental models)?");
  return params;
}

ComputeEigenstrainBase::ComputeEigenstrainBase(const InputParameters & parameters) :
    Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _incremental_form(getParam<bool>("incremental_form")),
    _eigenstrain(declareProperty<RankTwoTensor>(_base_name + "stress_free_strain")),
    _eigenstrain_old(_incremental_form ? &declarePropertyOld<RankTwoTensor>(_base_name + "stress_free_strain") : NULL),
    _eigenstrain_increment(_incremental_form ? &declareProperty<RankTwoTensor>(_base_name + "stress_free_strain_increment") : NULL)
{
}

void
ComputeEigenstrainBase::initQpStatefulProperties()
{
  if (_incremental_form)
    _eigenstrain[_qp].zero();
}

void
ComputeEigenstrainBase::computeQpProperties()
{
  computeQpEigenstrain();

  if (_incremental_form)
    (*_eigenstrain_increment)[_qp] = _eigenstrain[_qp] - (*_eigenstrain_old)[_qp];
}
