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

#include "NonlocalIntegratedBC.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<NonlocalIntegratedBC>()
{
  InputParameters params = validParams<IntegratedBC>();
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
NonlocalIntegratedBC::computeJacobianBlock(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    MooseVariable & jv = _sys.getVariable(_tid, jvar);
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), jvar);

    for (_j = 0; _j < _phi.size();
         _j++) // looping order for _i & _j are reversed for performance improvement
    {
      getUserObjectJacobian(jvar, jv.dofIndices()[_j]);
      for (_i = 0; _i < _test.size(); _i++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar);
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
    MooseVariable & jv = _sys.getVariable(_tid, jvar);
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
