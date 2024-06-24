//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsBasePD.h"

InputParameters
MechanicsBasePD::validParams()
{
  InputParameters params = PeridynamicsKernelBase::validParams();
  params.addClassDescription(
      "Base class for calculating the residual and Jacobian for the peridynamic mechanic kernels");

  params.addCoupledVar("temperature", "Nonlinear variable name for the temperature");
  params.addCoupledVar("out_of_plane_strain",
                       "Nonlinear variable name for the out_of_plane strain for "
                       "plane stress analysis using NOSPD formulation");

  return params;
}

MechanicsBasePD::MechanicsBasePD(const InputParameters & parameters)
  : DerivativeMaterialInterface<PeridynamicsKernelBase>(parameters),
    _temp_coupled(isCoupled("temperature")),
    _temp_var(_temp_coupled ? getVar("temperature", 0) : nullptr),
    _ndisp(coupledComponents("displacements")),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain_var(_out_of_plane_strain_coupled ? getVar("out_of_plane_strain", 0)
                                                          : nullptr),
    _orientation(nullptr)
{
  if (_ndisp != _dim)
    mooseError("Number of displacements should be consistent with mesh dimension!");

  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var.push_back(getVar("displacements", i));
}

void
MechanicsBasePD::initialSetup()
{
  _orientation = &_assembly.getFE(FEType(), 1)->get_dxyzdxi();
}

void
MechanicsBasePD::prepare()
{
  PeridynamicsKernelBase::prepare();

  _ivardofs.resize(_nnodes);
  _weights.resize(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    _ivardofs[nd] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    _weights[nd] = _pdmesh.getNeighborWeight(
        _current_elem->node_id(nd),
        _pdmesh.getNeighborIndex(_current_elem->node_id(nd), _current_elem->node_id(1 - nd)));
  }

  for (unsigned int i = 0; i < _dim; ++i)
    _current_vec(i) = _origin_vec(i) + _disp_var[i]->getNodalValue(*_current_elem->node_ptr(1)) -
                      _disp_var[i]->getNodalValue(*_current_elem->node_ptr(0));

  _current_unit_vec = _current_vec / _current_vec.norm();
}

void
MechanicsBasePD::computeOffDiagJacobian(const unsigned int jvar_num)
{
  prepare();

  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    unsigned int coupled_component = 0;
    bool active = false;

    for (unsigned int i = 0; i < _dim; ++i)
      if (jvar_num == _disp_var[i]->number())
      {
        coupled_component = i;
        active = true;
      }

    if (_temp_coupled && jvar_num == _temp_var->number())
      active = true;

    if (_out_of_plane_strain_coupled && jvar_num == _out_of_plane_strain_var->number())
      active = true;

    if (active)
    {
      prepareMatrixTag(_assembly, _var.number(), jvar_num);
      computeLocalOffDiagJacobian(jvar_num, coupled_component);
      accumulateTaggedLocalMatrix();

      if (_use_full_jacobian)
        computePDNonlocalOffDiagJacobian(jvar_num, coupled_component);
    }
  }
}
