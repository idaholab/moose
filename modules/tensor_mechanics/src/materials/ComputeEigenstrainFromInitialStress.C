/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeEigenstrainFromInitialStress.h"
#include "RankTwoTensor.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeEigenstrainFromInitialStress>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
  params.addClassDescription("Computes an eigenstrain from an initial stress");
  params.addRequiredParam<std::vector<FunctionName>>(
      "initial_stress",
      "A list of functions describing the initial stress.  If provided, there "
      "must be 9 of these, corresponding to the xx, yx, zx, xy, yy, zy, xz, yz, "
      "zz components respectively.  To compute the eigenstrain correctly, your "
      "elasticity tensor should not be time-varying in the first timestep");
  params.addParam<std::string>("base_name",
                               "The base_name for the elasticity tensor that will be "
                               "used to compute strain from stress.  Do not provide "
                               "any base_name if your elasticity tensor does not use "
                               "one.");
  params.set<bool>("incremental_form") = true;
  return params;
}

ComputeEigenstrainFromInitialStress::ComputeEigenstrainFromInitialStress(
    const InputParameters & parameters)
  : ComputeEigenstrainBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor"))
{
  if (_incremental_form == false)
    mooseError("ComputeEigenstrainFromInitialStress: incremental_form must be set to true");

  const std::vector<FunctionName> & fcn_names(
      getParam<std::vector<FunctionName>>("initial_stress"));
  const unsigned num = fcn_names.size();

  if (num != LIBMESH_DIM * LIBMESH_DIM)
    mooseError("ComputeEigenstrainFromInitialStress: ",
               LIBMESH_DIM * LIBMESH_DIM,
               " initial stress functions must be provided.  You supplied ",
               num,
               "\n");

  _initial_stress_fcn.resize(num);
  for (unsigned i = 0; i < num; ++i)
    _initial_stress_fcn[i] = &getFunctionByName(fcn_names[i]);
}

void
ComputeEigenstrainFromInitialStress::computeQpEigenstrain()
{
  if (_t_step == 1)
  {
    RankTwoTensor initial_stress;
    for (unsigned i = 0; i < LIBMESH_DIM; ++i)
      for (unsigned j = 0; j < LIBMESH_DIM; ++j)
        initial_stress(i, j) = _initial_stress_fcn[i * LIBMESH_DIM + j]->value(_t, _q_point[_qp]);

    _eigenstrain[_qp] = -_elasticity_tensor[_qp].invSymm() * initial_stress;
  }
  else
    _eigenstrain[_qp] = (*_eigenstrain_old)[_qp];
}
