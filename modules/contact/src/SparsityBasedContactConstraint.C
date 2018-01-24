//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SparsityBasedContactConstraint.h"

// MOOSE includes
#include "MooseVariable.h"

#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_matrix.h"

template <>
InputParameters
validParams<SparsityBasedContactConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  return params;
}

void
SparsityBasedContactConstraint::getConnectedDofIndices()
{
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3, 3, 0)
  _connected_dof_indices.clear();

  // An ugly hack: have to extract the row and look at it's sparsity structure, since otherwise I
  // won't get the dofs connected to this row by virtue of intervariable coupling
  // This is easier than sifting through all coupled variables, selecting those active on the
  // current subdomain, dealing with the scalar variables, etc.
  // Also, importantly, this will miss coupling to variables that might have introduced by prior
  // constraints similar to this one!
  PetscMatrix<Number> * petsc_jacobian = dynamic_cast<PetscMatrix<Number> *>(_jacobian);
  mooseAssert(petsc_jacobian, "Expected a PETSc matrix");
  Mat jac = petsc_jacobian->mat();
  PetscErrorCode ierr;
  PetscInt ncols;
  const PetscInt * cols;
  ierr = MatGetRow(jac, static_cast<PetscInt>(_var.nodalDofIndex()), &ncols, &cols, PETSC_NULL);
  CHKERRABORT(_communicator.get(), ierr);
  bool debug = false;
  if (debug)
  {
    libMesh::out << "_connected_dof_indices: adding " << ncols << " dofs from Jacobian row["
                 << _var.nodalDofIndex() << "] = [";
  }
  for (PetscInt i = 0; i < ncols; ++i)
  {
    if (debug)
    {
      libMesh::out << cols[i] << " ";
    }
    _connected_dof_indices.push_back(cols[i]);
  }
  if (debug)
  {
    libMesh::out << "]\n";
  }
  ierr = MatRestoreRow(jac, static_cast<PetscInt>(_var.nodalDofIndex()), &ncols, &cols, PETSC_NULL);
  CHKERRABORT(_communicator.get(), ierr);
#else
  NodeFaceConstraint::getConnectedDofIndices();
#endif
}
