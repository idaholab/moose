//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Libmesh includes
#include "MooseTypes.h"
#include "MooseMesh.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

namespace NS
{
namespace FV
{
/**
 * Relax the matrix to ensure diagonal dominance, we hold onto the difference in diagonals
 * for later use in relaxing the right hand side. For the details of this relaxation process, see
 *
 * Juretic, Franjo. Error analysis in finite volume CFD. Diss.
 * Imperial College London (University of London), 2005.
 *
 * @param matrix_in The matrix that needs to be relaxed
 * @param relaxation_parameter The scale which described how much the matrix is relaxed
 * @param diff_diagonal A vector holding the $A_{diag}-A_{diag, relaxed}$ entries for further
 *                      use in the relaxation of the right hand side
 */
void relaxMatrix(SparseMatrix<Number> & matrix_in,
                 const Real relaxation_parameter,
                 NumericVector<Number> & diff_diagonal);

/**
 * Relax the right hand side of an equation, this needs to be called once and the system matrix
 * has been relaxed and the field describing the difference in the diagonals of the system matrix
 * is already available. The relaxation process needs modification to both the system matrix and
 * the right hand side. For more information see:
 *
 * Juretic, Franjo. Error analysis in finite volume CFD. Diss.
 * Imperial College London (University of London), 2005.
 *
 * @param rhs_in The unrelaxed right hand side that needs to be relaxed
 * @param solution_in The solution
 * @param diff_diagonal The diagonal correction used for the corresponding system matrix
 */
void relaxRightHandSide(NumericVector<Number> & rhs_in,
                        const NumericVector<Number> & solution_in,
                        const NumericVector<Number> & diff_diagonal);

/**
 * Relax the update on a solution field using the following approach:
 * $u = u_{old}+\lambda (u - u_{old})$
 *
 * @param vector_new The new solution vector
 * @param vec_old The old solution vector
 * @param relaxation_factor The lambda parameter in the expression above
 */
void relaxSolutionUpdate(NumericVector<Number> & vec_new,
                         const NumericVector<Number> & vec_old,
                         const Real relaxation_factor);

/**
 * Limit a solution to its minimum and maximum bounds:
 * $u = min(max(u, min_limit), max_limit)$
 *
 * @param system_in The system whose solution shall be limited
 * @param min_limit = 0.0 The minimum limit for the solution
 * @param max_limit = 1e10 The maximum limit for the solution
 */
void limitSolutionUpdate(NumericVector<Number> & solution,
                         const Real min_limit = std::numeric_limits<Real>::epsilon(),
                         const Real max_limit = 1e10);

/**
 * Compute a normalization factor which is applied to the linear residual to determine
 * convergence. This function is based on the description provided here:
 *  // @article{greenshields2022notes,
 * title={Notes on computational fluid dynamics: General principles},
 * author={Greenshields, Christopher J and Weller, Henry G},
 * journal={(No Title)},
 * year={2022}
 * }
 * @param solution The solution vector
 * @param mat The system matrix
 * @param rhs The system right hand side
 */
Real computeNormalizationFactor(const NumericVector<Number> & solution,
                                const SparseMatrix<Number> & mat,
                                const NumericVector<Number> & rhs);

/**
 * Implicitly constrain the system by adding a factor*(u-u_desired) to it at a desired dof
 * value. To make sure the conditioning of the matrix does not change significantly, factor
 * is chosen to be the diagonal component of the matrix coefficients for a given dof.
 * @param mx The matrix of the system which needs to be constrained
 * @param rhs The right hand side of the system which needs to be constrained
 * @param value The desired value for the solution field at a dof
 * @param dof_id The ID of the dof which needs to be constrained
 */
void constrainSystem(SparseMatrix<Number> & mx,
                     NumericVector<Number> & rhs,
                     const Real desired_value,
                     const dof_id_type dof_id);

/**
 * Find the ID of the degree of freedom which corresponds to the variable and
 * a given point on the mesh
 * @param variable Reference to the moose variable whose dof should be fetched
 * @param mesh The moose mesh where the element is
 * @param point The point on the mesh
 */
dof_id_type findPointDoFID(const MooseVariableFieldBase & variable,
                           const MooseMesh & mesh,
                           const Point & point);

/**
 * Based on the residuals, determine if the iterative process converged or not
 * @param residuals The current (number of iterations, residual) pairs
 * @param abs_tolerances The corresponding absolute tolerances.
 */
bool converged(const std::vector<std::pair<unsigned int, Real>> & residuals,
               const std::vector<Real> & abs_tolerances);
} // End FV namespace
} // End Moose namespace
