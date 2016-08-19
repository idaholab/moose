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

#include "ArrayNodalBC.h"
#include "MooseVariable.h"
#include "Assembly.h"

// libMesh
#include "libmesh/petsc_vector.h"

template<>
InputParameters validParams<ArrayNodalBC>()
{
  InputParameters params = validParams<NodalBCBase>();

  return params;
}


ArrayNodalBC::ArrayNodalBC(const InputParameters & parameters) :
    NodalBCBase(parameters),
    _array_var(dynamic_cast<ArrayMooseVariable &>(_var)),
    _current_node(_array_var.node()),
    _residual(NULL, 0),
    _u(_array_var.nodalSln())
{
}

void
ArrayNodalBC::computeResidual(NumericVector<Number> & residual)
{
  if (_array_var.isNodalDefined())
  {
    auto & petsc_residual = dynamic_cast<PetscVector<Number> &>(residual);

    // We know we are dealing with a locally owned node
    // Therefore this processor also owns all of the degrees of freedom on that node
    // Therefore we can compute and set the residual directly for all of those degrees of freedom
    //
    // To do that we're going to:
    // 1. Get the global dof_index for the first variable
    // 2. Get the local dof_index for the first variable
    // 3. Get the raw PETSc data array
    // 4. Wrap the part of the raw residual vector covered by these dofs in an Eigen Vector
    // 5. Compute the residual for every variable in the array directly in place
    // 6. ???
    // 7. Profit!!!

    // 1. This is the global dof index for the first variable
    auto dof_idx = _array_var.nodalDofIndex();

    // 2. Map it to the local dof index:
    auto local_dof_idx = petsc_residual.map_global_to_local_index(dof_idx);

    // 3. Grab the raw PETSc data array
    PetscScalar * residual_values = const_cast<PetscScalar *>(petsc_residual.get_array());

    // 4. Wrap up the the raw residual vector in an Eigen Map
    new (&_residual) Eigen::Map<Eigen::VectorXd>(residual_values + local_dof_idx, _array_var.count());

    // 5. Compute the residual directly into _residual
    _qp = 0;
    computeQpResidual();

    // 6. ???
    petsc_residual.restore_array();

    // 7. Profit!!!
  }
}

void
ArrayNodalBC::computeJacobian()
{
  /*
  // We call the user's computeQpJacobian() function and store the
  // results in the _assembly object. We can't store them directly in
  // the element stiffness matrix, as they will only be inserted after
  // all the assembly is done.
  if (_moose_var.isNodalDefined())
  {
    _qp = 0;
    Real cached_val = computeQpJacobian();
    dof_id_type cached_row = _moose_var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_row, cached_val);

    if (_has_diag_save_in)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (unsigned int i=0; i<_diag_save_in.size(); i++)
        _diag_save_in[i]->sys().solution().set(_diag_save_in[i]->nodalDofIndex(), cached_val);
    }
  }
  */
}

void
ArrayNodalBC::computeOffDiagJacobian(unsigned int jvar)
{
  /*
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    _qp = 0;
    Real cached_val = computeQpOffDiagJacobian(jvar);
    dof_id_type cached_row = _moose_var.nodalDofIndex();
    // Note: this only works for Lagrange variables...
    dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

    // Cache the user's computeQpJacobian() value for later use.
    _fe_problem.assembly(0).cacheJacobianContribution(cached_row, cached_col, cached_val);
  }
  */
}


Real
ArrayNodalBC::computeQpJacobian()
{
  return 1.;
}

Real
ArrayNodalBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
