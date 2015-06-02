/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeStressBase.h"

template<>
InputParameters validParams<ComputeStressBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::vector<FunctionName> >("initial_stress", "A list of functions describing the initial stress.  If provided, there must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, zz components respectively.  If not provided, all components of the initial stress will be zero");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  return params;
}

ComputeStressBase::ComputeStressBase(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain(declareProperty<RankTwoTensor>(_base_name + "elastic_strain")),
    _elasticity_tensor(getMaterialProperty<ElasticityTensorR4>(_base_name + "elasticity_tensor")),
    _extra_stress(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "extra_stress")),
    _Jacobian_mult(declareProperty<ElasticityTensorR4>(_base_name + "Jacobian_mult"))
{
  const std::vector<FunctionName> & fcn_names(getParam<std::vector<FunctionName> >("initial_stress"));
  const unsigned num = fcn_names.size();

  if (!(num == 0 || num == 3*3))
    mooseError("Either zero or " << 3*3 << " initial stress functions must be provided to TensorMechanicsMaterial.  You supplied " << num << "\n");

  _initial_stress.resize(num);
  for (unsigned i = 0 ; i < num ; ++i)
    _initial_stress[i] = &getFunctionByName(fcn_names[i]);
}

void
ComputeStressBase::initQpStatefulProperties()
{
  _stress[_qp].zero();
  if (_initial_stress.size() == 3*3)
    for (unsigned i = 0 ; i < 3 ; ++i)
      for (unsigned j = 0 ; j < 3 ; ++j)
        _stress[_qp](i, j) = _initial_stress[i*3 + j]->value(_t, _q_point[_qp]);
  _elastic_strain[_qp].zero();
}

void
ComputeStressBase::computeQpProperties()
{
  computeQpStress();

  //Add in extra stress
  _stress[_qp] += _extra_stress[_qp];
}
