//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsBasePD.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<MechanicsBasePD>()
{
  InputParameters params = validParams<KernelBasePD>();
  params.addClassDescription(
      "Base class for calculating residual and Jacobian for peridynamic mechanic kernels");

  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable names for the displacements");
  params.addParam<NonlinearVariableName>("temperature",
                                         "Nonlinear variable name for the temperature");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains to be coupled in non-ordinary state-based mechanics kernels");
  params.addParam<NonlinearVariableName>("out_of_plane_strain",
                                         "Nonlinear variable name for the out_of_plane strain for "
                                         "plane stress analysis using SNOSPD formulation");

  return params;
}

MechanicsBasePD::MechanicsBasePD(const InputParameters & parameters)
  : DerivativeMaterialInterface<KernelBasePD>(parameters),
    _temp_coupled(isParamValid("temperature")),
    _temp_var(_temp_coupled
                  ? &_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("temperature"))
                  : NULL),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _deigenstrain_dT(_eigenstrain_names.size()),
    _out_of_plane_strain_coupled(isParamValid("out_of_plane_strain")),
    _out_of_plane_strain_var(
        _out_of_plane_strain_coupled
            ? &_subproblem.getVariable(_tid, getParam<NonlinearVariableName>("out_of_plane_strain"))
            : NULL),
    _orientation(NULL)
{
  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));

  if (nl_vnames.size() != _dim)
    mooseError("Number of displacements should be consistent with mesh dimension!");

  for (unsigned int i = 0; i < _dim; ++i)
    _disp_var.push_back(&_subproblem.getVariable(_tid, nl_vnames[i]));

  for (unsigned int i = 0; i < _deigenstrain_dT.size(); ++i)
    _deigenstrain_dT[i] =
        &getMaterialPropertyDerivative<RankTwoTensor>(_eigenstrain_names[i], _temp_var->name());
}

void
MechanicsBasePD::initialSetup()
{
  _orientation = &_assembly.getFE(FEType(), 1)->get_dxyzdxi();
}

void
MechanicsBasePD::prepare()
{
  KernelBasePD::prepare();

  _ivardofs_ij.resize(_nnodes);

  for (unsigned int i = 0; i < _nnodes; ++i)
    _ivardofs_ij[i] = _nodes_ij[i]->dof_number(_sys.number(), _var.number(), 0);

  for (unsigned int i = 0; i < _dim; ++i)
    _cur_ori_ij(i) = _origin_vec_ij(i) + _disp_var[i]->getNodalValue(*_nodes_ij[1]) -
                     _disp_var[i]->getNodalValue(*_nodes_ij[0]);

  _cur_len_ij = _cur_ori_ij.norm();
  _cur_ori_ij /= _cur_len_ij;
}

void
MechanicsBasePD::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  prepare();

  if (jvar.number() == _var.number())
    computeJacobian();
  else
  {
    unsigned int coupled_component = 0;
    bool active = false;

    for (unsigned int i = 0; i < _dim; ++i)
      if (jvar.number() == _disp_var[i]->number())
      {
        coupled_component = i;
        active = true;
      }

    if (_temp_coupled && jvar.number() == _temp_var->number())
    {
      coupled_component = 3;
      active = true;
    }

    if (_out_of_plane_strain_coupled && jvar.number() == _out_of_plane_strain_var->number())
    {
      coupled_component = 4;
      active = true;
    }

    if (active)
    {
      DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar.number());
      _local_ke.resize(ke.m(), ke.n());
      _local_ke.zero();

      computeLocalOffDiagJacobian(coupled_component);

      ke += _local_ke;

      if (_use_full_jacobian)
        computePDNonlocalOffDiagJacobian(jvar.number(), coupled_component);
    }
  }
}
