//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStressBase.h"
#include "ComputeElasticityTensorBase.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeStressBase>()
{
  InputParameters params = validParams<Material>();
  params.addDeprecatedParam<std::vector<FunctionName>>(
      "initial_stress",
      "A list of functions describing the initial stress.  If provided, there "
      "must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, "
      "zz components respectively.  If not provided, all components of the "
      "initial stress will be zero",
      "This functionality was deprecated on 12 October 2017 and is set to be"
      "removed on 12 March 2018.  Please use ComputeEigenstrainFromInitialStress"
      "instead");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>("store_stress_old",
                        false,
                        "Parameter which indicates whether the old "
                        "stress state, required for the HHT time "
                        "integration scheme and Rayleigh damping, needs "
                        "to be stored");
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ComputeStressBase::ComputeStressBase(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain(declareProperty<RankTwoTensor>(_base_name + "elastic_strain")),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _extra_stress(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "extra_stress")),
    _Jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _store_stress_old(getParam<bool>("store_stress_old")),
    // InitialStress Deprecation: remove the following
    _initial_stress_provided(getParam<std::vector<FunctionName>>("initial_stress").size() ==
                             LIBMESH_DIM * LIBMESH_DIM),
    _initial_stress(_initial_stress_provided
                        ? &declareProperty<RankTwoTensor>(_base_name + "initial_stress")
                        : nullptr),
    _initial_stress_old(_initial_stress_provided
                            ? &getMaterialPropertyOld<RankTwoTensor>(_base_name + "initial_stress")
                            : nullptr)
{

  if (getParam<bool>("use_displaced_mesh"))
    mooseError("The stress calculator needs to run on the undisplaced mesh.");

  // InitialStress Deprecation: remove following initial stress stuff
  const std::vector<FunctionName> & fcn_names(
      getParam<std::vector<FunctionName>>("initial_stress"));
  const unsigned num = fcn_names.size();

  if (!(num == 0 || num == LIBMESH_DIM * LIBMESH_DIM))
    mooseError("Either zero or ",
               LIBMESH_DIM * LIBMESH_DIM,
               " initial stress functions must be provided.  You supplied ",
               num,
               "\n");

  _initial_stress_fcn.resize(num);
  for (unsigned i = 0; i < num; ++i)
    _initial_stress_fcn[i] = &getFunctionByName(fcn_names[i]);
}

void
ComputeStressBase::initQpStatefulProperties()
{
  _elastic_strain[_qp].zero();
  _stress[_qp].zero();
  // InitialStress Deprecation: remove the following
  if (_initial_stress_provided)
  {
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      for (unsigned j = 0; j < LIBMESH_DIM; ++j)
        (*_initial_stress)[_qp](i, j) =
            _initial_stress_fcn[i * LIBMESH_DIM + j]->value(_t, _q_point[_qp]);
  }
  addQpInitialStress();
}

void
ComputeStressBase::computeQpProperties()
{
  // InitialStress Deprecation: remove the following 2 lines
  if (_initial_stress_provided)
    (*_initial_stress)[_qp] = (*_initial_stress_old)[_qp];

  computeQpStress();

  // Add in extra stress
  _stress[_qp] += _extra_stress[_qp];
}

void
ComputeStressBase::addQpInitialStress()
{
  if (_initial_stress_provided)
    _stress[_qp] += (*_initial_stress)[_qp];
}
