//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PrintMatricesNSProblem.h"
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
#include <slepceps.h>

registerMooseObject("NavierStokesTestApp", PrintMatricesNSProblem);

InputParameters
PrintMatricesNSProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addRequiredParam<TagName>(
      "pressure_mass_matrix", "The matrix tag name corresponding to the pressure mass matrix.");
  params.addRequiredParam<TagName>(
      "velocity_mass_matrix", "The matrix tag name corresponding to the velocity mass matrix.");
  params.addParam<std::vector<TagName>>(
      "jump_matrices",
      {},
      "The matrices corresponding to different (superpositions) of finite element weak forms");
  params.addRequiredParam<NonlinearVariableName>("u", "The interior x-velocity variable");
  params.addRequiredParam<NonlinearVariableName>("v", "The interior y-velocity variable");
  params.addParam<NonlinearVariableName>(NS::pressure, "The pressure in the volume");
  params.addParam<NonlinearVariableName>(NS::pressure + "_bar", "The pressure on the facets");
  params.addParam<bool>("print", true, "Whether to print the matrices");
  return params;
}

PrintMatricesNSProblem::PrintMatricesNSProblem(const InputParameters & parameters)
  : FEProblem(parameters),
    _pressure_mass_matrix(getParam<TagName>("pressure_mass_matrix")),
    _velocity_mass_matrix(getParam<TagName>("velocity_mass_matrix")),
    _jump_matrices(getParam<std::vector<TagName>>("jump_matrices"))
{
  if (_communicator.size() > 1)
    mooseError("This problem can only be used in serial");
}

void
PrintMatricesNSProblem::onTimestepEnd()
{
  FEProblem::onTimestepEnd();

  const bool print = getParam<bool>("print");

  const bool has_p = isParamValid(NS::pressure);
  const bool has_pbar = isParamValid(NS::pressure + "_bar");

  auto & nl = getNonlinearSystemBase(0);
  auto & dof_map = nl.dofMap();
  auto & lm_mesh = mesh().getMesh();

  const auto & u_var = nl.getVariable(0, getParam<NonlinearVariableName>("u"));
  const auto & v_var = nl.getVariable(0, getParam<NonlinearVariableName>("v"));
  const MooseVariableFieldBase * p_var = nullptr;
  const MooseVariableFieldBase * pb_var = nullptr;
  if (has_p)
    p_var = &nl.getVariable(0, getParam<NonlinearVariableName>(NS::pressure));
  if (has_pbar)
    pb_var = &nl.getVariable(0, getParam<NonlinearVariableName>(NS::pressure + "_bar"));

  std::vector<dof_id_type> u_indices, v_indices, p_vol_indices, pb_indices, vel_indices, p_indices;
  dof_map.local_variable_indices(u_indices, lm_mesh, u_var.number());
  dof_map.local_variable_indices(v_indices, lm_mesh, v_var.number());
  if (has_p)
    dof_map.local_variable_indices(p_vol_indices, lm_mesh, p_var->number());
  if (has_pbar)
    dof_map.local_variable_indices(pb_indices, lm_mesh, pb_var->number());
  vel_indices = u_indices;
  vel_indices.insert(vel_indices.end(), v_indices.begin(), v_indices.end());
  p_indices = p_vol_indices;
  if (has_pbar)
    p_indices.insert(p_indices.end(), pb_indices.begin(), pb_indices.end());

  PetscMatrix<Number> vel_p_mat(_communicator), p_vel_mat(_communicator), p_mass_mat(_communicator),
      vel_mass_mat(_communicator);
  const auto pressure_mass_matrix_tag_id = getMatrixTagID(_pressure_mass_matrix);
  auto & system_size_pressure_mass_matrix = nl.getMatrix(pressure_mass_matrix_tag_id);
  const auto velocity_mass_matrix_tag_id = getMatrixTagID(_velocity_mass_matrix);
  auto & system_size_velocity_mass_matrix = nl.getMatrix(velocity_mass_matrix_tag_id);
  auto * const system_matrix =
      dynamic_cast<PetscMatrix<Number> *>(&nl.nonlinearSolver()->system().get_system_matrix());
  mooseAssert(system_matrix, "Must be a PETSc matrix");

  auto write_matrix = [this](Mat write_mat, const std::string & mat_name)
  {
    PetscViewer matviewer;
    LibmeshPetscCallA(
        _communicator.get(),
        PetscViewerBinaryOpen(_communicator.get(), mat_name.c_str(), FILE_MODE_WRITE, &matviewer));
    LibmeshPetscCallA(_communicator.get(), MatView(write_mat, matviewer));
    LibmeshPetscCallA(_communicator.get(), PetscViewerDestroy(&matviewer));
  };

  auto do_vel_p =
      [this,
       write_matrix,
       print,
       &vel_indices,
       &system_size_pressure_mass_matrix,
       &system_size_velocity_mass_matrix,
       system_matrix,
       &vel_p_mat,
       &p_vel_mat,
       &p_mass_mat,
       &vel_mass_mat](const auto & pressure_indices, const std::string & outer_matrix_name)
  {
    auto compute_triple_product_matrix =
        [this, print, write_matrix](PetscMatrix<Number> & lhs,
                                    PetscMatrix<Number> & mass_matrix,
                                    PetscMatrix<Number> & rhs,
                                    const std::string & inner_matrix_name)
    {
      Mat B, M, Minv;

      // Create B
      LibmeshPetscCall(MatCreateDense(_communicator.get(),
                                      mass_matrix.local_m(),
                                      mass_matrix.local_n(),
                                      mass_matrix.m(),
                                      mass_matrix.n(),
                                      nullptr,
                                      &B));
      const PetscScalar one = 1.0;
      for (const auto i : make_range(mass_matrix.row_start(), mass_matrix.row_stop()))
      {
        const auto petsc_i = cast_int<PetscInt>(i);
        LibmeshPetscCall(MatSetValues(B, 1, &petsc_i, 1, &petsc_i, &one, INSERT_VALUES));
      }
      LibmeshPetscCall(MatAssemblyBegin(B, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCall(MatAssemblyEnd(B, MAT_FINAL_ASSEMBLY));

      // Create Minv
      LibmeshPetscCall(MatCreateDense(_communicator.get(),
                                      mass_matrix.local_m(),
                                      mass_matrix.local_n(),
                                      mass_matrix.m(),
                                      mass_matrix.n(),
                                      nullptr,
                                      &Minv));

      // Factor mass matrix
      LibmeshPetscCall(MatConvert(mass_matrix.mat(), MATDENSE, MAT_INITIAL_MATRIX, &M));
      LibmeshPetscCall(MatLUFactor(M, nullptr, nullptr, nullptr));

      // Solve for Minv
      LibmeshPetscCall(MatMatSolve(M, B, Minv));

      //
      // Compute triple product and write the result
      //

      Mat triple_product_mat;
      LibmeshPetscCall(MatMatMatMult(
          lhs.mat(), Minv, rhs.mat(), MAT_INITIAL_MATRIX, PETSC_DEFAULT, &triple_product_mat));

      PetscMatrix<Number> triple_product(triple_product_mat, _communicator);
      if (print)
      {
        _console << std::endl << "Printing the '" << inner_matrix_name << "' matrix" << std::endl;
        triple_product.print();
      }
      write_matrix(triple_product.mat(), inner_matrix_name + std::string(".mat"));

      LibmeshPetscCall(MatDestroy(&triple_product_mat));
      LibmeshPetscCall(MatDestroy(&B));
      LibmeshPetscCall(MatDestroy(&M));
      LibmeshPetscCall(MatDestroy(&Minv));
    };

    system_size_pressure_mass_matrix.create_submatrix(
        p_mass_mat, pressure_indices, pressure_indices);
    system_size_velocity_mass_matrix.create_submatrix(vel_mass_mat, vel_indices, vel_indices);
    system_matrix->create_submatrix(vel_p_mat, vel_indices, pressure_indices);
    system_matrix->create_submatrix(p_vel_mat, pressure_indices, vel_indices);

    compute_triple_product_matrix(
        vel_p_mat, p_mass_mat, p_vel_mat, outer_matrix_name + "_grad_div");
    compute_triple_product_matrix(
        p_vel_mat, vel_mass_mat, vel_p_mat, outer_matrix_name + "_div_grad");
  };

  if (has_p)
    do_vel_p(p_vol_indices, "vel_p");
  if (has_pbar)
    do_vel_p(pb_indices, "vel_pb");
  do_vel_p(p_indices, "vel_all_p");

  for (const auto & jump_name : _jump_matrices)
  {
    PetscMatrix<Number> jump_mat(_communicator);
    const auto jump_matrix_tag_id = getMatrixTagID(jump_name);
    auto & system_size_jump_matrix = nl.getMatrix(jump_matrix_tag_id);
    system_size_jump_matrix.create_submatrix(jump_mat, vel_indices, vel_indices);
    if (print)
    {
      _console << std::endl << "Printing the jump matrix '" << jump_name << "'" << std::endl;
      jump_mat.print();
    }
    write_matrix(jump_mat.mat(), jump_name + std::string(".mat"));
  }
}
