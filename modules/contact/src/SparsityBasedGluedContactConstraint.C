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
#include "SparsityBasedGluedContactConstraint.h"


// libMesh includes
#include "libmesh/petsc_macro.h"
#include "libmesh/petsc_matrix.h"


template<>
InputParameters validParams<SparsityBasedGluedContactConstraint>()
{
  InputParameters params = validParams<GluedContactConstraint>();
  return params;
}



void
SparsityBasedGluedContactConstraint::computeJacobian()
{

  _connected_dof_indices.clear();

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
  // An ugly hack: have to extract the row and look at it's sparsity structure, since otherwise I won't get the dofs connected to this row by virtue of intervariable coupling
  // This is easier than sifting through all coupled variables, selecting those active on the current subdomain, dealing with the scalar variables, etc.
  // Also, importantly, this will miss coupling to variables that might have introduced by prior constraints similar to this one!
  PetscMatrix<Number>* petsc_jacobian = dynamic_cast<PetscMatrix<Number> *>(_jacobian);
  mooseAssert(petsc_jacobian, "Expected a PETSc matrix");
  Mat jac = petsc_jacobian->mat();
  PetscErrorCode ierr;
  PetscInt ncols;
  const PetscInt *cols;
  ierr = MatGetRow(jac,_var.nodalDofIndex(),&ncols,&cols,PETSC_NULL);CHKERRABORT(libMesh::COMM_WORLD, ierr);
  bool debug = false;
  if(debug) {
    libMesh::out << "_connected_dof_indices: adding " << ncols << " dofs from Jacobian row[" << _var.nodalDofIndex() << "] = [";
  }
  for (PetscInt i = 0; i < ncols; ++i) {
    if(debug) {
      libMesh::out << cols[i] << " ";
    }
    _connected_dof_indices.push_back(cols[i]);
  }
  if (debug) {
    libMesh::out << "]\n";
  }
  ierr = MatRestoreRow(jac,_var.nodalDofIndex(),&ncols,&cols,PETSC_NULL);CHKERRABORT(libMesh::COMM_WORLD, ierr);
#else

  std::vector<unsigned int> & elems = _node_to_elem_map[_current_node->id()];
  std::set<unsigned int> unique_dof_indices;

  // Get the dof indices from each elem connected to the node
  for(unsigned int el=0; el < elems.size(); ++el)
  {
    unsigned int cur_elem = elems[el];

    std::vector<unsigned int> dof_indices;
    _var.getDofIndices(_mesh.elem(cur_elem), dof_indices);

    for(unsigned int di=0; di < dof_indices.size(); di++)
      unique_dof_indices.insert(dof_indices[di]);
  }

  for(std::set<unsigned int>::iterator sit=unique_dof_indices.begin(); sit != unique_dof_indices.end(); ++sit)
    _connected_dof_indices.push_back(*sit);
#endif
  //  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.index(), _var.index());

  //  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), _var.number());
  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.index(), _var.index());

  _Kee.resize(_test_slave.size(), _connected_dof_indices.size());
  _Kne.resize(_test_master.size(), _connected_dof_indices.size());

  _phi_slave.resize(_connected_dof_indices.size());

  _qp = 0;

  // Fill up _phi_slave so that it is 1 when j corresponds to this dof and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for(unsigned int j=0; j<_connected_dof_indices.size(); j++)
  {
    _phi_slave[j].resize(1);

    if(_connected_dof_indices[j] == _var.nodalDofIndex())
      _phi_slave[j][_qp] = 1.0;
    else
      _phi_slave[j][_qp] = 0.0;
  }

  for (_i = 0; _i < _test_slave.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kee(_i,_j) += computeQpJacobian(Moose::SlaveSlave);

  for (_i=0; _i<_test_slave.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Ken(_i,_j) += computeQpJacobian(Moose::SlaveMaster);

  for (_i=0; _i<_test_master.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j=0; _j<_connected_dof_indices.size(); _j++)
      _Kne(_i,_j) += computeQpJacobian(Moose::MasterSlave);

  for (_i=0; _i<_test_master.size(); _i++)
    for (_j=0; _j<_phi_master.size(); _j++)
      Knn(_i,_j) += computeQpJacobian(Moose::MasterMaster);
}

