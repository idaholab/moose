//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainFromInitialStress.h"
#include "RankTwoTensor.h"
#include "Function.h"
#include "Conversion.h" // for stringify

registerMooseObject("TensorMechanicsApp", ComputeEigenstrainFromInitialStress);

InputParameters
ComputeEigenstrainFromInitialStress::validParams()
{
  InputParameters params = ComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes an eigenstrain from an initial stress");
  params.addRequiredParam<std::vector<FunctionName>>(
      "initial_stress",
      "A list of functions describing the initial stress.  There must be 9 of these, corresponding "
      "to the xx, yx, zx, xy, yy, zy, xz, yz, zz components respectively.  To compute the "
      "eigenstrain correctly, your elasticity tensor should not be time-varying in the first "
      "timestep");
  params.addCoupledVar("initial_stress_aux",
                       "A list of 9 AuxVariables describing the initial stress.  If provided, each "
                       "of these is multiplied by its corresponding initial_stress function to "
                       "obtain the relevant component of initial stress.");
  params.addParam<std::string>("base_name",
                               "The base_name for the elasticity tensor that will be "
                               "used to compute strain from stress.  Do not provide "
                               "any base_name if your elasticity tensor does not use "
                               "one.");
  return params;
}

ComputeEigenstrainFromInitialStress::ComputeEigenstrainFromInitialStress(
    const InputParameters & parameters)
  : ComputeEigenstrainBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _eigenstrain_old(getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_name)),
    _ini_aux_provided(isParamValid("initial_stress_aux")),
    _ini_aux(_ini_aux_provided ? coupledValues("initial_stress_aux")
                               : std::vector<const VariableValue *>())
{
  const std::vector<FunctionName> & fcn_names(
      getParam<std::vector<FunctionName>>("initial_stress"));
  const std::size_t num = fcn_names.size();

  if (num != LIBMESH_DIM * LIBMESH_DIM)
    paramError(
        "initial_stress",
        "ComputeEigenstrainFromInitialStress: " + Moose::stringify(LIBMESH_DIM * LIBMESH_DIM) +
            " initial stress functions must be provided.  You supplied " + Moose::stringify(num) +
            "\n");

  _initial_stress_fcn.resize(num);
  for (unsigned i = 0; i < num; ++i)
    _initial_stress_fcn[i] = &getFunctionByName(fcn_names[i]);

  if (_ini_aux_provided)
  {
    const std::size_t aux_size = coupledComponents("initial_stress_aux");
    if (aux_size != LIBMESH_DIM * LIBMESH_DIM)
      paramError("initial_stress_aux",
                 "ComputeEigenstrainFromInitialStress: If you supply initial_stress_aux, " +
                     Moose::stringify(LIBMESH_DIM * LIBMESH_DIM) +
                     " values must be given.  You supplied " + Moose::stringify(aux_size) + "\n");
  }
}

void
ComputeEigenstrainFromInitialStress::computeQpEigenstrain()
{
  if (_t_step == 1)
  {
    RankTwoTensor initial_stress;
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      {
        initial_stress(i, j) = _initial_stress_fcn[i * LIBMESH_DIM + j]->value(_t, _q_point[_qp]);
        if (_ini_aux_provided)
          initial_stress(i, j) *= (*_ini_aux[i * LIBMESH_DIM + j])[_qp];
      }

    _eigenstrain[_qp] = -_elasticity_tensor[_qp].invSymm() * initial_stress;
  }
  else
    _eigenstrain[_qp] = _eigenstrain_old[_qp];
}
