//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGNavierStokesProblem.h"
#include "NonlinearSystemBase.h"
#include "NS.h"
#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/nonlinear_implicit_system.h"
#include <petscmat.h>

registerMooseObject("NavierStokesApp", IPHDGNavierStokesProblem);

InputParameters
IPHDGNavierStokesProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addRequiredParam<TagName>("mass_matrix",
                                   "The matrix tag name corresponding to the mass matrix.");
  params.addRequiredParam<std::vector<TagName>>(
      "jump_matrices",
      "The matrices corresponding to different (superpositions) of finite element weak forms");
  params.addRequiredParam<NonlinearVariableName>("u", "The interior x-velocity variable");
  params.addRequiredParam<NonlinearVariableName>("v", "The interior y-velocity variable");
  params.addRequiredParam<NonlinearVariableName>(NS::pressure, "The pressure in the volume");
  params.addRequiredParam<NonlinearVariableName>(NS::pressure + "_bar",
                                                 "The pressure on the facets");
  params.addParam<bool>("print", true, "Whether to print the matrices");
  return params;
}

IPHDGNavierStokesProblem::IPHDGNavierStokesProblem(const InputParameters & parameters)
  : FEProblem(parameters),
    _mass_matrix(getParam<TagName>("mass_matrix")),
    _jump_matrices(getParam<std::vector<TagName>>("jump_matrices"))
{
}

void
IPHDGNavierStokesProblem::onTimestepEnd()
{
  FEProblem::onTimestepEnd();

  if (!getParam<bool>("print"))
    return;

  auto & nl = getNonlinearSystemBase(0);
  auto & dof_map = nl.dofMap();
  auto & lm_mesh = mesh().getMesh();

  const auto & u_var = nl.getVariable(0, getParam<NonlinearVariableName>("u"));
  const auto & v_var = nl.getVariable(0, getParam<NonlinearVariableName>("v"));
  const auto & p_var = nl.getVariable(0, getParam<NonlinearVariableName>(NS::pressure));
  const auto & pb_var = nl.getVariable(0, getParam<NonlinearVariableName>(NS::pressure + "_bar"));

  std::vector<dof_id_type> u_indices, v_indices, p_vol_indices, pb_indices, vel_indices, p_indices;
  dof_map.local_variable_indices(u_indices, lm_mesh, u_var.number());
  dof_map.local_variable_indices(v_indices, lm_mesh, v_var.number());
  dof_map.local_variable_indices(p_vol_indices, lm_mesh, p_var.number());
  dof_map.local_variable_indices(pb_indices, lm_mesh, pb_var.number());
  vel_indices = u_indices;
  vel_indices.insert(vel_indices.end(), v_indices.begin(), v_indices.end());
  p_indices = p_vol_indices;
  p_indices.insert(p_indices.end(), pb_indices.begin(), pb_indices.end());

  PetscMatrix<Number> vel_p_mat(_communicator), p_vel_mat(_communicator), p_mass_mat(_communicator);
  const auto mass_matrix_tag_id = getMatrixTagID(_mass_matrix);
  auto & system_size_mass_matrix = nl.getMatrix(mass_matrix_tag_id);
  auto & system_matrix = nl.nonlinearSolver()->system().get_system_matrix();

  auto do_vel_p = [this,
                   &vel_indices,
                   &system_size_mass_matrix,
                   &system_matrix,
                   &vel_p_mat,
                   &p_vel_mat,
                   &p_mass_mat](const auto & pressure_indices, const auto & matrix_name)
  {
    system_size_mass_matrix.create_submatrix(p_mass_mat, pressure_indices, pressure_indices);
    system_matrix.create_submatrix(vel_p_mat, vel_indices, pressure_indices);
    system_matrix.create_submatrix(p_vel_mat, pressure_indices, vel_indices);

    PetscVector<Number> p_mass_mat_diag(_communicator);
    p_mass_mat_diag.init(p_mass_mat.m(), p_mass_mat.local_m(), false, PARALLEL);
    p_mass_mat.get_diagonal(p_mass_mat_diag);
    p_mass_mat_diag.reciprocal();

    auto ierr = MatDiagonalSet(p_mass_mat.mat(), p_mass_mat_diag.vec(), INSERT_VALUES);
    LIBMESH_CHKERR(ierr);

    Mat product_mat;
    ierr = MatMatMatMult(vel_p_mat.mat(),
                         p_mass_mat.mat(),
                         p_vel_mat.mat(),
                         MAT_INITIAL_MATRIX,
                         PETSC_DEFAULT,
                         &product_mat);
    LIBMESH_CHKERR(ierr);

    _console << std::endl << "Printing the '" << matrix_name << "' matrix" << std::endl;
    PetscMatrix<Number> product(product_mat, _communicator);
    product.print();
    ierr = MatDestroy(&product_mat);
    LIBMESH_CHKERR(ierr);
  };

  do_vel_p(pb_indices, "vel-pb");
  do_vel_p(p_indices, "vel-all-p");

  for (const auto & jump_name : _jump_matrices)
  {
    _console << std::endl << "Printing the jump matrix '" << jump_name << "'" << std::endl;
    PetscMatrix<Number> jump_mat(_communicator);
    const auto jump_matrix_tag_id = getMatrixTagID(jump_name);
    auto & system_size_jump_matrix = nl.getMatrix(jump_matrix_tag_id);
    system_size_jump_matrix.create_submatrix(jump_mat, vel_indices, vel_indices);
    jump_mat.print();
  }
}
