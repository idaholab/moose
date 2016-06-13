/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MooseVariableBase.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/variable.h"
#include "libmesh/dof_map.h"

MooseVariableBase::MooseVariableBase(unsigned int var_num, const FEType & fe_type, SystemBase & sys, Assembly & assembly, Moose::VarKindType var_kind) :
    _var_num(var_num),
    _fe_type(fe_type),
    _var_kind(var_kind),
    _subproblem(sys.subproblem()),
    _sys(sys),
    _variable(sys.system().variable(_var_num)),
    _assembly(assembly),
    _dof_map(sys.dofMap()),
    _mesh(_subproblem.mesh()),
    _scaling_factor(1.0)
{
}

MooseVariableBase::~MooseVariableBase()
{
}

const std::string &
MooseVariableBase::name() const
{
  return _sys.system().variable(_var_num).name();
}

std::vector<dof_id_type> &
MooseVariableBase::allDofIndices()
{
  std::vector<std::set<dof_id_type> > dofs(libMesh::n_threads());
  std::set<dof_id_type> all_dofs;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    for (unsigned int i = 0; i < _mesh.nElem(); ++i)
    {
      std::vector<dof_id_type> di;
      _dof_map.dof_indices(_mesh.elemPtr(i), di, _var_num);
      dofs[tid].insert(di.begin(), di.end());
    }
    all_dofs.insert(dofs[tid].begin(), dofs[tid].end());
  }
  _all_dof_indices.assign(all_dofs.begin(), all_dofs.end());

  return _all_dof_indices;
}

Order
MooseVariableBase::order() const
{
  return _fe_type.order;
}
