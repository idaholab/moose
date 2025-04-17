//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDistributedLoadShell.h"
#include "Assembly.h"
#include "Function.h"
#include "MooseError.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"

registerMooseObject("SolidMechanicsApp", ADDistributedLoadShell);
registerMooseObject("SolidMechanicsApp", ADDistributedLoadShell);

template <bool is_ad>
InputParameters
ADDistributedLoadShellTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription(
      "Applies a distributed load (specified in units of force per area) on the shell plane in a "
      "given direction (e.g. self_weight, wind load) or normal to the shell plan (e.g. pressure "
      "loads)");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addParam<bool>(
      "project_load_to_normal", false, "Whether to apply the distributed load normal to the shell");
  params.addRequiredParam<FunctionName>(
      "function",
      "The function that describes the distributed load specified in units of force per area ");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <bool is_ad>
ADDistributedLoadShellTempl<is_ad>::ADDistributedLoadShellTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _ndisp(this->coupledComponents("displacements")),
    _component(libMesh::invalid_uint),
    _function(&this->getFunction("function")),
    _project_load_to_normal(parameters.isParamSetByUser("project_load_to_normal")
                                ? this->template getParam<bool>("project_load_to_normal")
                                : false),
    _nodes(4),
    _use_displaced_mesh(this->template getParam<bool>("use_displaced_mesh"))
{
  if (_ndisp != 3)
    mooseError("ADDistributedLoadShell: parameter 'displacements' needs to have exactly three "
               "components.");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var.push_back(this->coupled("displacements", i));
    if (_var.number() == _disp_var[i])
    {
      _component = i;
    }
  }
  if (_component == libMesh::invalid_uint)
    mooseError("Problem with displacements in " + _name);
}

template <bool is_ad>
GenericReal<is_ad>
ADDistributedLoadShellTempl<is_ad>::computeQpResidual()
{
  for (unsigned int i = 0; i < 4; ++i)
    _nodes[i] = _current_elem->node_ptr(i);

  _v1 = (*_nodes[1] - *_nodes[0]);
  _v2 = (*_nodes[2] - *_nodes[0]);

  _normal = _v1.cross(_v2);
  _normal /= _normal.norm();

  if (_project_load_to_normal)
    return computeFactor() * (_normal(_component) * _test[_i][_qp]);
  else
    return computeFactor() * (_test[_i][_qp]);
}

template <bool is_ad>
GenericReal<is_ad>
ADDistributedLoadShellTempl<is_ad>::computeFactor() const
{
  return _function->value(_t, _q_point[_qp]);
}

template class ADDistributedLoadShellTempl<false>;
template class ADDistributedLoadShellTempl<true>;
