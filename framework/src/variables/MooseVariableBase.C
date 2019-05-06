//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableBase.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseMesh.h"

#include "libmesh/variable.h"
#include "libmesh/dof_map.h"
#include "libmesh/system.h"

MooseVariableBase::MooseVariableBase(unsigned int var_num,
                                     const FEType & fe_type,
                                     SystemBase & sys,
                                     Moose::VarKindType var_kind,
                                     THREAD_ID tid,
                                     unsigned int count)
  : _var_num(var_num),
    _fe_type(fe_type),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _variable(sys.system().variable(_var_num)),
    _dof_map(sys.dofMap()),
    _mesh(_subproblem.mesh()),
    _tid(tid),
    _count(count),
    _scaling_factor(std::vector<Real>(_count, 1.0))
{
  if (_count > 1)
  {
    auto name0 = _sys.system().variable(_var_num).name();
    std::size_t found = name0.find_last_of("_");
    if (found == std::string::npos)
      mooseError("");
    _name = name0.substr(0, found);
  }
  else
    _name = _sys.system().variable(_var_num).name();
}

MooseVariableBase::~MooseVariableBase() {}

const std::string &
MooseVariableBase::name() const
{
  return _name;
}

const std::vector<dof_id_type> &
MooseVariableBase::allDofIndices() const
{
  const auto it = _sys.subproblem()._var_dof_map.find(name());
  if (it != _sys.subproblem()._var_dof_map.end())
    return it->second;
  else
    mooseError("VariableAllDoFMap not prepared for ",
               name(),
               " . Check nonlocal coupling requirement for the variable.");
}

Order
MooseVariableBase::order() const
{
  return _fe_type.order;
}
