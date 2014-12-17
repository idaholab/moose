#include "ElasticEnergyMaterial.h"
#include "RankTwoTensor.h"

template<>
InputParameters validParams<ElasticEnergyMaterial>()
{
  InputParameters params = validParams<DerivativeBaseMaterial>();
  params.addClassDescription("Free energy material for the elastic energy contributions.");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

ElasticEnergyMaterial::ElasticEnergyMaterial(const std::string & name,
                                             InputParameters parameters) :
    DerivativeBaseMaterial(name, parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _strain(getMaterialProperty<RankTwoTensor>(_base_name + "elastic_strain"))
{
  _dstress.resize(_nargs);
  _d2stress.resize(_nargs);
  _dstrain.resize(_nargs);
  _d2strain.resize(_nargs);

  // fetch stress and strain derivatives (in simple eigenstrain models this is is only w.r.t. 'c')
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _dstress[i] = &getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "stress", _arg_names[i]);
    _dstrain[i] = &getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _arg_names[i]);

    _d2stress[i].resize(_nargs);
    _d2strain[i].resize(_nargs);

    for (unsigned int j = 0; j < _nargs; ++j)
    {
      _d2stress[i][j] = &getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "stress", _arg_names[i], _arg_names[j]);
      _d2strain[i][j] = &getMaterialPropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", _arg_names[i], _arg_names[j]);
    }
  }
}

Real
ElasticEnergyMaterial::computeF()
{
  return 0.5 * _stress[_qp].doubleContraction(_strain[_qp]);
}

Real
ElasticEnergyMaterial::computeDF(unsigned int i)
{
  return 0.5 * ((*_dstress[i])[_qp].doubleContraction(_strain[_qp])
                + _stress[_qp].doubleContraction((*_dstrain[i])[_qp]));
}

Real
ElasticEnergyMaterial::computeD2F(unsigned int i, unsigned int j)
{
  return 0.5 * ((*_d2stress[i][j])[_qp].doubleContraction(_strain[_qp])
                + (*_dstress[i])[_qp].doubleContraction((*_dstrain[j])[_qp])
                + (*_dstress[j])[_qp].doubleContraction((*_dstrain[i])[_qp])
                + _stress[_qp].doubleContraction((*_d2strain[i][j])[_qp]));
}
