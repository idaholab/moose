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

#include "NonlocalKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<NonlocalKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

NonlocalKernel::NonlocalKernel(const InputParameters & parameters) :
    Kernel(parameters)
{
}

void
NonlocalKernel::computeJacobian()
{
  Kernel::computeJacobian();

  DenseMatrix<Number> & keg = _assembly.jacobianBlockNonlocal(_var.number(), _var.number());
  _nonlocal_ke.resize(keg.m(), keg.n());
  _nonlocal_ke.zero();

  // compiling set of global IDs for the local DOFs on the element
  std::set<dof_id_type> local_dofindices(_var.dofIndices().begin(), _var.dofIndices().end());
  // storing the global IDs for all the DOFs of the variable
  std::vector<dof_id_type> var_alldofindices(_var.allDofIndices());
  unsigned int n_total_dofs = var_alldofindices.size();

  for (_i = 0; _i < _test.size(); _i++)
    for (_k = 0; _k < n_total_dofs; _k++)
    {
      auto it = local_dofindices.find(var_alldofindices[_k]);
      if (it == local_dofindices.end()) // eleminating the local components
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          _nonlocal_ke(_i, var_alldofindices[_k]) += _JxW[_qp] * _coord[_qp] * computeQpNonlocalJacobian(var_alldofindices[_k]);
    }
  keg += _nonlocal_ke;
}

void
NonlocalKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    Kernel::computeOffDiagJacobian(jvar);

    MooseVariable & jv = _sys.getVariable(_tid, jvar);
    DenseMatrix<Number> & keg = _assembly.jacobianBlockNonlocal(_var.number(), jvar);
    // compiling set of global IDs for the local DOFs on the element
    std::set<dof_id_type> local_dofindices(jv.dofIndices().begin(), jv.dofIndices().end());
    // storing the global IDs for all the DOFs of the variable
    std::vector<dof_id_type> jv_alldofindices(jv.allDofIndices());
    unsigned int n_total_dofs = jv_alldofindices.size();

    for (_i = 0; _i < _test.size(); _i++)
      for (_k = 0; _k < n_total_dofs; _k++)
      {
        auto it = local_dofindices.find(jv_alldofindices[_k]);
        if (it == local_dofindices.end()) // eleminating the local components
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            keg(_i, jv_alldofindices[_k]) += _JxW[_qp] * _coord[_qp] * computeQpNonlocalOffDiagJacobian(jvar, jv_alldofindices[_k]);
      }
  }
}

Real
NonlocalKernel::computeQpNonlocalJacobian(dof_id_type /*dof_index*/)
{
  return 0.0;
}

Real
NonlocalKernel::computeQpNonlocalOffDiagJacobian(unsigned int /*jvar*/, dof_id_type /*dof_index*/)
{
  return 0.0;
}
