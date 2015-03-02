#include "ComputeEigenStrainBase.h"

template<>
InputParameters validParams<ComputeEigenStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<bool>("incremental_form", false, "Should the Eigenstrain be in incremental form for finite strain methods?");
  return params;
}

ComputeEigenStrainBase::ComputeEigenStrainBase(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _incremental_form(getParam<bool>("incremental_form")),
    _eigen_strain(declareProperty<RankTwoTensor>(_base_name + "eigen_strain")),
    _eigen_strain_old(_incremental_form ? &declarePropertyOld<RankTwoTensor>(_base_name + "eigen_strain") : NULL),
    _eigen_strain_increment(declareProperty<RankTwoTensor>(_base_name + "eigen_strain_increment"))
{
}

void
ComputeEigenStrainBase::initQpStatefulProperties()
{
  _eigen_strain[_qp].zero();
  if (_incremental_form)
    (*_eigen_strain_old)[_qp] = _eigen_strain[_qp];

  _eigen_strain_increment[_qp].zero();
}

void
ComputeEigenStrainBase::computeQpProperties()
{
  computeQpEigenStrain();

  if (_incremental_form)
    _eigen_strain_increment[_qp] = _eigen_strain[_qp] - (*_eigen_strain_old)[_qp];
  else
    _eigen_strain_increment[_qp].zero();
}

