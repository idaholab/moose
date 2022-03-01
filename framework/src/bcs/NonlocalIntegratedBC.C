//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlocalIntegratedBC.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseMesh.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
NonlocalIntegratedBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  return params;
}

NonlocalIntegratedBC::NonlocalIntegratedBC(const InputParameters & parameters)
  : IntegratedBC(parameters)
{
  _mesh.errorIfDistributedMesh("NonlocalIntegratedBC");
  mooseWarning("NonlocalIntegratedBC is a computationally expensive experimental capability used "
               "only for integral terms.");
}

void
NonlocalIntegratedBC::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  _local_ke.resize(ke.m(), ke.n());
  _local_ke.zero();

  for (_j = 0; _j < _phi.size();
       _j++) // looping order for _i & _j are reversed for performance improvement
  {
    getUserObjectJacobian(_var.number(), _var.dofIndices()[_j]);
    for (_i = 0; _i < _test.size(); _i++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
  }

  ke += _local_ke;

  if (_has_diag_save_in)
  {
    unsigned int rows = ke.m();
    DenseVector<Number> diag(rows);
    for (unsigned int i = 0; i < rows; i++)
      diag(i) = _local_ke(i, i);

    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

void
NonlocalIntegratedBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    const auto & jvar = getVariable(jvar_num);
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar_num);

    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = jvar.dofIndices().size();

    for (_j = 0; _j < phi_size;
         _j++) // looping order for _i & _j are reversed for performance improvement
    {
      getUserObjectJacobian(jvar_num, jvar.dofIndices()[_j]);
      for (_i = 0; _i < _test.size(); _i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar.number());
    }
  }
}

void
NonlocalIntegratedBC::computeNonlocalJacobian()
{
  DenseMatrix<Number> & keg = _assembly.jacobianBlockNonlocal(_var.number(), _var.number());
  // compiling set of global IDs for the local DOFs on the element
  std::set<dof_id_type> local_dofindices(_var.dofIndices().begin(), _var.dofIndices().end());
  // storing the global IDs for all the DOFs of the variable
  const std::vector<dof_id_type> & var_alldofindices = _var.allDofIndices();
  unsigned int n_total_dofs = var_alldofindices.size();

  for (_k = 0; _k < n_total_dofs;
       _k++) // looping order for _i & _k are reversed for performance improvement
  {
    // eliminating the local components
    auto it = local_dofindices.find(var_alldofindices[_k]);
    if (it == local_dofindices.end())
    {
      getUserObjectJacobian(_var.number(), var_alldofindices[_k]);
      // skip global DOFs that do not contribute to the jacobian
      if (!globalDoFEnabled(_var, var_alldofindices[_k]))
        continue;

      for (_i = 0; _i < _test.size(); _i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          keg(_i, _k) += _JxW[_qp] * _coord[_qp] * computeQpNonlocalJacobian(var_alldofindices[_k]);
    }
  }
}

void
NonlocalIntegratedBC::computeNonlocalOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeNonlocalJacobian();
  else
  {
    MooseVariableFEBase & jv = _sys.getVariable(_tid, jvar);
    DenseMatrix<Number> & keg = _assembly.jacobianBlockNonlocal(_var.number(), jvar);
    // compiling set of global IDs for the local DOFs on the element
    std::set<dof_id_type> local_dofindices(jv.dofIndices().begin(), jv.dofIndices().end());
    // storing the global IDs for all the DOFs of the variable
    const std::vector<dof_id_type> & jv_alldofindices = jv.allDofIndices();
    unsigned int n_total_dofs = jv_alldofindices.size();

    for (_k = 0; _k < n_total_dofs;
         _k++) // looping order for _i & _k are reversed for performance improvement
    {
      // eliminating the local components
      auto it = local_dofindices.find(jv_alldofindices[_k]);
      if (it == local_dofindices.end())
      {
        getUserObjectJacobian(jvar, jv_alldofindices[_k]);
        // skip global DOFs that do not contribute to the jacobian
        if (!globalDoFEnabled(jv, jv_alldofindices[_k]))
          continue;

        for (_i = 0; _i < _test.size(); _i++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            keg(_i, _k) += _JxW[_qp] * _coord[_qp] *
                           computeQpNonlocalOffDiagJacobian(jvar, jv_alldofindices[_k]);
      }
    }
  }
}
