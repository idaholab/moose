/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AnisotropicElasticityTensor.h"

AnisotropicElasticityTensor::AnisotropicElasticityTensor()
  : ElasticityTensor(false), _euler_angle(3), _c11(0), _c12(0), _c44(0)
{
}

void
AnisotropicElasticityTensor::setFirstEulerAngle(const Real a1)
{
  _euler_angle[0] = a1;
}

void
AnisotropicElasticityTensor::setSecondEulerAngle(const Real a2)
{
  _euler_angle[1] = a2;
}

void
AnisotropicElasticityTensor::setThirdEulerAngle(const Real a3)
{
  _euler_angle[2] = a3;
}

void
AnisotropicElasticityTensor::setMaterialConstantc11(const Real c11)
{
  _c11 = c11;
}

void
AnisotropicElasticityTensor::setMaterialConstantc12(const Real c12)
{
  _c12 = c12;
}

void
AnisotropicElasticityTensor::setMaterialConstantc44(const Real c44)
{
  _c44 = c44;
}

void
AnisotropicElasticityTensor::calculateEntries(unsigned int /*qp*/)
{

  // form_r_matrix();
  // Form rotation matrix from Euler angles
  const Real phi1 = _euler_angle[0] * (M_PI / 180.0);
  const Real phi = _euler_angle[1] * (M_PI / 180.0);
  const Real phi2 = _euler_angle[2] * (M_PI / 180.0);

  Real cp1 = cos(phi1);
  Real cp2 = cos(phi2);
  Real cp = cos(phi);

  Real sp1 = sin(phi1);
  Real sp2 = sin(phi2);
  Real sp = sin(phi);

  DenseMatrix<Real> R; // Rotational Matrix
  R(0, 0) = cp1 * cp2 - sp1 * sp2 * cp;
  R(0, 1) = sp1 * cp2 + cp1 * sp2 * cp;
  R(0, 2) = sp2 * sp;
  R(1, 0) = -cp1 * sp2 - sp1 * cp2 * cp;
  R(1, 1) = -sp1 * sp2 + cp1 * cp2 * cp;
  R(1, 2) = cp2 * sp;
  R(2, 0) = sp1 * sp;
  R(2, 1) = -cp1 * sp;
  R(2, 2) = cp;

  // Initialize material Dt matrix
  DenseMatrix<Real> Dt; // 6 x 6 Material Matrix
  Dt(0, 0) = Dt(1, 1) = Dt(2, 2) = _c11;
  Dt(0, 1) = Dt(0, 2) = Dt(1, 0) = Dt(2, 0) = Dt(1, 2) = Dt(2, 1) = _c12;
  Dt(3, 3) = Dt(4, 4) = Dt(5, 5) = 2 * _c44;

  // Form Q = R dyadic R and Q transpose
  DenseMatrix<Real> Q, Qt; // Q = R (dyadic) R and Q transpose
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
          Q(((i * 3) + k), ((j * 3) + l)) = R(i, j) * R(k, l);

  for (unsigned int p = 0; p < 9; p++)
    for (unsigned int q = 0; q < 9; q++)
      Qt(q, p) = Q(p, q);

  // Form two kinds of transformation matrix
  // TransD6toD9 transfroms Dt[6][6] to Dmat[9][9]
  // TransD9toD6 transforms Dmat[9][9] to Dt[6][6]
  DenseMatrix<Real> trans_d6_to_d9, transpose_trans_d6_to_d9;
  DenseMatrix<Real> trans_d9_to_d6, transpose_trans_d9_to_d6;
  Real sqrt2 = std::sqrt(2.0);
  Real a = 1 / sqrt2;

  trans_d6_to_d9(0, 0) = trans_d6_to_d9(4, 1) = trans_d6_to_d9(8, 2) = 1.0;
  trans_d6_to_d9(1, 3) = trans_d6_to_d9(3, 3) = a;
  trans_d6_to_d9(2, 4) = trans_d6_to_d9(6, 4) = a;
  trans_d6_to_d9(5, 5) = trans_d6_to_d9(7, 5) = a;

  for (unsigned int i = 0; i < 9; i++)
    for (unsigned int j = 0; j < 6; j++)
      transpose_trans_d6_to_d9(j, i) = trans_d6_to_d9(i, j);

  trans_d9_to_d6(0, 0) = trans_d9_to_d6(1, 4) = trans_d9_to_d6(2, 8) = 1.0;
  trans_d9_to_d6(3, 3) = trans_d9_to_d6(4, 6) = trans_d9_to_d6(5, 7) = sqrt2;

  for (unsigned int i = 0; i < 6; i++)
    for (unsigned int j = 0; j < 9; j++)
      transpose_trans_d9_to_d6(j, i) = trans_d9_to_d6(i, j);

  // The function makes use of TransD6toD9 matrix to transfrom Dt[6][6] to Dmat[9][9]
  // Dmat = T * Dt * TT

  DenseMatrix<Real> outputMatrix(9, 6);

  outputMatrix = trans_d6_to_d9;
  outputMatrix.right_multiply(Dt);

  DenseMatrix<Real> Dmat; // 9 x 9 Material Matrix
  Dmat = outputMatrix;
  Dmat.right_multiply(transpose_trans_d6_to_d9);

  // The function makes use of Q matrix to rotate Dmat[9][9] to QDmat[9][9]
  // QDmat = QT * Dmat * Q

  DenseMatrix<Real> outputMatrix99(9, 9);

  outputMatrix99 = Qt;
  outputMatrix99.right_multiply(Dmat);

  DenseMatrix<Real> QDmat; // Rotated Material Matrix
  QDmat = outputMatrix99;
  QDmat.right_multiply(Q);

  // Convert 9X9 matrix QDmat to 81 vector

  unsigned int count = 0;

  for (unsigned int j = 0; j < 9; j++)
    for (unsigned int i = 0; i < 9; i++)
    {
      _values[count] = QDmat(i, j);
      count = count + 1;
    }
}
