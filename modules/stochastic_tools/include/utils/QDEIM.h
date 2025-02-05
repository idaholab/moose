///* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <MooseTypes.h>

// Algorithm is derived from this resource.
// Z. Drmač and S. Gugercin, "A New Selection Operator for the Discrete Empirical Interpolation
// Method---Improved A Priori Error Bound and Extensions," SIAM Journal on Scientific Computing,
// vol. 38, no. 2, pp. A631-A648, 2016, doi: 10.1137/15M1019271. [Online]. Available:
// https://doi.org/10.1137/15M1019271

class QDEIM
{
public:
  /**
   * Constructor for QDEIM.
   * @param ortho_basis_vecs Vector of orthonormal basis vectors
   */
  QDEIM(std::vector<DenseVector<double>> & ortho_basis_vecs);

  /**
   * Computes the selection indices using the QDEIM algorithm.
   * @param selection_indices Vector to store the computed selection indices
   */
  void computeSelection(std::vector<dof_id_type> & selection_indices);

  /**
   * Computes both the selection indices and a new basis using the QDEIM algorithm.
   * @param new_basis Pointer to a vector to store the computed new basis vectors
   * @param selection_indices Vector to store the computed selection indices
   */
  void computeSelectionAndBasis(std::vector<DenseVector<double>> * new_basis,
                                std::vector<dof_id_type> & selection_indices);

private:
  /**
   * Builds the orthonormal matrix from the basis vectors.
   */
  void buildOrthoMatrix();

  /**
   * Performs QR decomposition on the orthonormal matrix.
   */
  void performQRDecomposition();

  /**
   * Extracts the selection indices from the QR decomposition.
   */
  void extractIndices();

  /**
   * Constructs the projection matrix using the selected indices.
   */
  void constructProjectionMatrix();

  /**
   * Creates the inverse permutation vector.
   */
  void createInversePermutationVector();

  /**
   * Applies the inverse permutation to the selection vector.
   */
  void applyInversePermutation();

  /**
   * Converts an Eigen vector to a std::vector.
   * @param sample_indices Vector to store the converted indices
   */
  void convertEigenVectorToStdVector(std::vector<dof_id_type> & sample_indices);

  /**
   * Converts an Eigen matrix to a std::vector of DenseVector.
   * @param new_basis Vector to store the converted basis vectors
   */
  void convertEigenMatrixToStdVector(std::vector<DenseVector<double>> & new_basis);

  /// Vector of orthonormal basis vectors
  std::vector<DenseVector<double>> & _ortho_basis_vecs;

  /// Number of basis vectors
  dof_id_type _num_basis;

  /// Length of each basis vector
  dof_id_type _length_basis;

  /// Orthonormal basis matrix
  RealEigenMatrix _ortho_basis;

  /// QR decomposition object
  Eigen::ColPivHouseholderQR<RealEigenMatrix> _qr;

  /// Vector of selected indices
  Eigen::VectorXi _indices;

  /// Selection vector
  Eigen::VectorXi _selection_vector;

  /// Projection matrix
  Eigen::MatrixXd _projection_mat;
};
