//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeStressBase.h"
#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "DelimitedFileReader.h"
#include "MooseVariableBase.h"
// #include "RankTwoTensorImplementation.h"



template <typename R2>
InputParameters
ADComputeStressBaseTempl<R2>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.suppressParameter<bool>("use_displaced_mesh");
  params.addParam<std::vector<MaterialPropertyName>>(
      "extra_stress_names",
      std::vector<MaterialPropertyName>(),
      "Material property names of rank two tensors to be added to the stress.");
  return params;
}

template <typename R2>
ADComputeStressBaseTempl<R2>::ADComputeStressBaseTempl(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _mechanical_strain(getADMaterialProperty<R2>(_base_name + "mechanical_strain")),
    _stress(declareADProperty<R2>(_base_name + "stress")),
    _elastic_strain(declareADProperty<R2>(_base_name + "elastic_strain")),
    _extra_stresses(getParam<std::vector<MaterialPropertyName>>("extra_stress_names").size()),
    _functions00(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_00")),
    _functions10(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_10")),
    _functions20(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_20")),
    _functions01(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_01")),
    _functions11(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_11")),
    _functions21(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_21")),
    _functions02(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_02")),
    _functions12(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_12")),
    _functions22(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_stress_22")),
    _functions00es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_00")),
    _functions10es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_10")),
    _functions20es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_20")),
    _functions01es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_01")),
    _functions11es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_11")),
    _functions21es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_21")),
    _functions02es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_02")),
    _functions12es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_12")),
    _functions22es(FunctionInterface::getFunctionByName(_base_name + "solution_fcn_elastic_strain_22"))
{
  if (getParam<bool>("use_displaced_mesh"))
    mooseError("The stress calculator needs to run on the undisplaced mesh.");

  const std::vector<MaterialPropertyName> extra_stress_names =
      getParam<std::vector<MaterialPropertyName>>("extra_stress_names");
  for (MooseIndex(_extra_stresses) i = 0; i < _extra_stresses.size(); ++i)
    _extra_stresses[i] = &getMaterialProperty<R2>(extra_stress_names[i]);
}

template <typename R2>
void
ADComputeStressBaseTempl<R2>::initQpStatefulProperties()
{
  _initstress.fillFromInputVector(std::vector<Real>{_functions00.value(0, _q_point[_qp]), _functions11.value(0, _q_point[_qp]), _functions22.value(0, _q_point[_qp]), _functions12.value(0, _q_point[_qp]), _functions02.value(0, _q_point[_qp]), _functions01.value(0, _q_point[_qp])});
  _initstrain.fillFromInputVector(std::vector<Real>{_functions00es.value(0, _q_point[_qp]), _functions11es.value(0, _q_point[_qp]), _functions22es.value(0, _q_point[_qp]), _functions12es.value(0, _q_point[_qp]), _functions02es.value(0, _q_point[_qp]), _functions01es.value(0, _q_point[_qp])});

  
  _elastic_strain[_qp].zero();
  _elastic_strain[_qp] = _initstrain;
  _stress[_qp].zero();
  _stress[_qp] = _initstress;
}

template <typename R2>
void
ADComputeStressBaseTempl<R2>::computeQpProperties()
{
  computeQpStress();

  // Add in extra stress
  for (MooseIndex(_extra_stresses) i = 0; i < _extra_stresses.size(); ++i)
    _stress[_qp] += (*_extra_stresses[i])[_qp];
}

template class ADComputeStressBaseTempl<RankTwoTensor>;
template class ADComputeStressBaseTempl<SymmetricRankTwoTensor>;
