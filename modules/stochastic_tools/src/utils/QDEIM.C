//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QDEIM.h"
#include "libmesh/int_range.h"
#include <vector>

QDEIM::QDEIM(std::vector<DenseVector<double>> & ortho_basis_vecs)
  : _ortho_basis_vecs(ortho_basis_vecs)
{
}

void
QDEIM::computeSelection(std::vector<dof_id_type> & selection_indices)
{
  // Determine the number and lengths of the basis
  _num_basis = _ortho_basis_vecs.size();
  _length_basis = (_num_basis > 0) ? _ortho_basis_vecs[0].size() : 0;

  buildOrthoMatrix();
  performQRDecomposition();
  extractIndices();

  convertEigenVectorToStdVector(selection_indices);
}

void
QDEIM::computeSelectionAndBasis(std::vector<DenseVector<double>> * new_basis,
                                std::vector<dof_id_type> & selection_indices)
{

  computeSelection(selection_indices);

  constructProjectionMatrix();
  applyInversePermutation();

  // convert eigen vector to std vector
  // Convert _M to new_basis
  convertEigenMatrixToStdVector(*new_basis);
}

void
QDEIM::buildOrthoMatrix()
{
  _ortho_basis.resize(_num_basis, _length_basis);

  // We expect the we got a vector of DenseVectors where each vector made up from
  // rows of the ortho-normal basis matrix. For the QDEIM algorithm, we need the
  // transpose of that matrix.
  for (const auto i : make_range(_num_basis))
  {
    _ortho_basis.row(i) =
        Eigen::Map<const Eigen::VectorXd>(_ortho_basis_vecs[i].get_values().data(), _length_basis);
  }
}

void
QDEIM::performQRDecomposition()
{
  _qr = _ortho_basis.colPivHouseholderQr();
}

void
QDEIM::extractIndices()
{
  // Get the column permutation matrix from QR decomposition
  auto perm = _qr.colsPermutation();
  _indices = perm.indices();

  // Extract the first _num_cols indices (pivotal indices)
  _selection_vector = _indices.head(_num_basis);
}

void
QDEIM::constructProjectionMatrix()
{
  // Construct the matrix M using the QR decomposition results

  auto R = _qr.matrixR().topLeftCorner(_num_basis, _num_basis);
  auto rightSide = _qr.matrixR().topRightCorner(_num_basis, _length_basis - _num_basis);
  _projection_mat = Eigen::MatrixXd::Identity(_length_basis, _num_basis);
  _projection_mat.bottomRows(_length_basis - _num_basis) =
      R.triangularView<Eigen::Upper>().solve(rightSide).transpose();
}

void
QDEIM::applyInversePermutation()
{
  // Create the inverse permutation vector
  Eigen::VectorXi Pinverse(_length_basis);
  for (const auto i : make_range(_length_basis))
    Pinverse(_indices(i)) = i;

  Eigen::MatrixXd mat_permuted(_length_basis, _num_basis);
  for (const auto i : make_range(_length_basis))
    mat_permuted.row(Pinverse(i)) = _projection_mat.row(i);

  _projection_mat = mat_permuted;
}

void
QDEIM::convertEigenMatrixToStdVector(std::vector<DenseVector<double>> & new_basis)
{

  new_basis.resize(_projection_mat.rows()); // Reserve space for rows

  for (const auto i : make_range(_projection_mat.rows()))
  {
    DenseVector<double> rowVector(_projection_mat.cols()); // Create a dense vector for each row

    for (const auto j : make_range(_projection_mat.cols()))
      rowVector(j) = _projection_mat(i, j); // Copy each element

    new_basis[i] = rowVector; // Add the row vector to new_basis
  }
}

void
QDEIM::convertEigenVectorToStdVector(std::vector<dof_id_type> & vector)
{
  vector.resize(_selection_vector.size()); // Reserve space for elements

  for (const auto i : make_range(_selection_vector.size()))
    vector[i] = static_cast<dof_id_type>(_selection_vector(i));
}
