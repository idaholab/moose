/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Element.h"
#include "Nonlinear3D.h"
#include "SolidModel.h"

namespace SolidMechanics
{

Element::Element(SolidModel & solid_model,
                 const std::string & /*name*/,
                 const InputParameters & parameters)
  : Coupleable(&solid_model, false), ZeroInterface(parameters), _solid_model(solid_model)
{
}

////////////////////////////////////////////////////////////////////////

Element::~Element() {}

////////////////////////////////////////////////////////////////////////

Real
Element::detMatrix(const ColumnMajorMatrix & A)
{
  mooseAssert(A.n() == 3 && A.m() == 3, "detMatrix requires 3x3 matrix");

  Real Axx = A(0, 0);
  Real Axy = A(0, 1);
  Real Axz = A(0, 2);
  Real Ayx = A(1, 0);
  Real Ayy = A(1, 1);
  Real Ayz = A(1, 2);
  Real Azx = A(2, 0);
  Real Azy = A(2, 1);
  Real Azz = A(2, 2);

  return Axx * Ayy * Azz + Axy * Ayz * Azx + Axz * Ayx * Azy - Azx * Ayy * Axz - Azy * Ayz * Axx -
         Azz * Ayx * Axy;
}

////////////////////////////////////////////////////////////////////////

void
Element::invertMatrix(const ColumnMajorMatrix & A, ColumnMajorMatrix & Ainv)
{
  Real Axx = A(0, 0);
  Real Axy = A(0, 1);
  Real Axz = A(0, 2);
  Real Ayx = A(1, 0);
  Real Ayy = A(1, 1);
  Real Ayz = A(1, 2);
  Real Azx = A(2, 0);
  Real Azy = A(2, 1);
  Real Azz = A(2, 2);

  mooseAssert(detMatrix(A) > 0, "Matrix is not positive definite!");
  Real detInv = 1 / detMatrix(A);

  Ainv(0, 0) = +(Ayy * Azz - Azy * Ayz) * detInv;
  Ainv(0, 1) = -(Axy * Azz - Azy * Axz) * detInv;
  Ainv(0, 2) = +(Axy * Ayz - Ayy * Axz) * detInv;
  Ainv(1, 0) = -(Ayx * Azz - Azx * Ayz) * detInv;
  Ainv(1, 1) = +(Axx * Azz - Azx * Axz) * detInv;
  Ainv(1, 2) = -(Axx * Ayz - Ayx * Axz) * detInv;
  Ainv(2, 0) = +(Ayx * Azy - Azx * Ayy) * detInv;
  Ainv(2, 1) = -(Axx * Azy - Azx * Axy) * detInv;
  Ainv(2, 2) = +(Axx * Ayy - Ayx * Axy) * detInv;
}

////////////////////////////////////////////////////////////////////////

void
Element::rotateSymmetricTensor(const ColumnMajorMatrix & R,
                               const RealTensorValue & T,
                               RealTensorValue & result)
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0, 0) * T(0, 0) + R(0, 1) * T(1, 0) + R(0, 2) * T(2, 0);
  const Real T01 = R(0, 0) * T(0, 1) + R(0, 1) * T(1, 1) + R(0, 2) * T(2, 1);
  const Real T02 = R(0, 0) * T(0, 2) + R(0, 1) * T(1, 2) + R(0, 2) * T(2, 2);

  const Real T10 = R(1, 0) * T(0, 0) + R(1, 1) * T(1, 0) + R(1, 2) * T(2, 0);
  const Real T11 = R(1, 0) * T(0, 1) + R(1, 1) * T(1, 1) + R(1, 2) * T(2, 1);
  const Real T12 = R(1, 0) * T(0, 2) + R(1, 1) * T(1, 2) + R(1, 2) * T(2, 2);

  const Real T20 = R(2, 0) * T(0, 0) + R(2, 1) * T(1, 0) + R(2, 2) * T(2, 0);
  const Real T21 = R(2, 0) * T(0, 1) + R(2, 1) * T(1, 1) + R(2, 2) * T(2, 1);
  const Real T22 = R(2, 0) * T(0, 2) + R(2, 1) * T(1, 2) + R(2, 2) * T(2, 2);

  result = RealTensorValue(T00 * R(0, 0) + T01 * R(0, 1) + T02 * R(0, 2),
                           T00 * R(1, 0) + T01 * R(1, 1) + T02 * R(1, 2),
                           T00 * R(2, 0) + T01 * R(2, 1) + T02 * R(2, 2),

                           T10 * R(0, 0) + T11 * R(0, 1) + T12 * R(0, 2),
                           T10 * R(1, 0) + T11 * R(1, 1) + T12 * R(1, 2),
                           T10 * R(2, 0) + T11 * R(2, 1) + T12 * R(2, 2),

                           T20 * R(0, 0) + T21 * R(0, 1) + T22 * R(0, 2),
                           T20 * R(1, 0) + T21 * R(1, 1) + T22 * R(1, 2),
                           T20 * R(2, 0) + T21 * R(2, 1) + T22 * R(2, 2));
}

////////////////////////////////////////////////////////////////////////

void
Element::rotateSymmetricTensor(const ColumnMajorMatrix & R,
                               const SymmTensor & T,
                               SymmTensor & result)
{

  //     R           T         Rt
  //  00 01 02   00 01 02   00 10 20
  //  10 11 12 * 10 11 12 * 01 11 21
  //  20 21 22   20 21 22   02 12 22
  //
  const Real T00 = R(0, 0) * T.xx() + R(0, 1) * T.xy() + R(0, 2) * T.zx();
  const Real T01 = R(0, 0) * T.xy() + R(0, 1) * T.yy() + R(0, 2) * T.yz();
  const Real T02 = R(0, 0) * T.zx() + R(0, 1) * T.yz() + R(0, 2) * T.zz();

  const Real T10 = R(1, 0) * T.xx() + R(1, 1) * T.xy() + R(1, 2) * T.zx();
  const Real T11 = R(1, 0) * T.xy() + R(1, 1) * T.yy() + R(1, 2) * T.yz();
  const Real T12 = R(1, 0) * T.zx() + R(1, 1) * T.yz() + R(1, 2) * T.zz();

  const Real T20 = R(2, 0) * T.xx() + R(2, 1) * T.xy() + R(2, 2) * T.zx();
  const Real T21 = R(2, 0) * T.xy() + R(2, 1) * T.yy() + R(2, 2) * T.yz();
  const Real T22 = R(2, 0) * T.zx() + R(2, 1) * T.yz() + R(2, 2) * T.zz();

  result.xx(T00 * R(0, 0) + T01 * R(0, 1) + T02 * R(0, 2));
  result.yy(T10 * R(1, 0) + T11 * R(1, 1) + T12 * R(1, 2));
  result.zz(T20 * R(2, 0) + T21 * R(2, 1) + T22 * R(2, 2));
  result.xy(T00 * R(1, 0) + T01 * R(1, 1) + T02 * R(1, 2));
  result.yz(T10 * R(2, 0) + T11 * R(2, 1) + T12 * R(2, 2));
  result.zx(T00 * R(2, 0) + T01 * R(2, 1) + T02 * R(2, 2));
}

////////////////////////////////////////////////////////////////////////

void
Element::unrotateSymmetricTensor(const ColumnMajorMatrix & R,
                                 const SymmTensor & T,
                                 SymmTensor & result)
{

  //     Rt           T         R
  //  00 10 20    00 01 02   00 01 02
  //  01 11 21  * 10 11 12 * 10 11 12
  //  02 12 22    20 21 22   20 21 22
  //
  const Real T00 = R(0, 0) * T.xx() + R(1, 0) * T.xy() + R(2, 0) * T.zx();
  const Real T01 = R(0, 0) * T.xy() + R(1, 0) * T.yy() + R(2, 0) * T.yz();
  const Real T02 = R(0, 0) * T.zx() + R(1, 0) * T.yz() + R(2, 0) * T.zz();

  const Real T10 = R(0, 1) * T.xx() + R(1, 1) * T.xy() + R(2, 1) * T.zx();
  const Real T11 = R(0, 1) * T.xy() + R(1, 1) * T.yy() + R(2, 1) * T.yz();
  const Real T12 = R(0, 1) * T.zx() + R(1, 1) * T.yz() + R(2, 1) * T.zz();

  const Real T20 = R(0, 2) * T.xx() + R(1, 2) * T.xy() + R(2, 2) * T.zx();
  const Real T21 = R(0, 2) * T.xy() + R(1, 2) * T.yy() + R(2, 2) * T.yz();
  const Real T22 = R(0, 2) * T.zx() + R(1, 2) * T.yz() + R(2, 2) * T.zz();

  result.xx(T00 * R(0, 0) + T01 * R(1, 0) + T02 * R(2, 0));
  result.yy(T10 * R(0, 1) + T11 * R(1, 1) + T12 * R(2, 1));
  result.zz(T20 * R(0, 2) + T21 * R(1, 2) + T22 * R(2, 2));
  result.xy(T00 * R(0, 1) + T01 * R(1, 1) + T02 * R(2, 1));
  result.yz(T10 * R(0, 2) + T11 * R(1, 2) + T12 * R(2, 2));
  result.zx(T00 * R(0, 2) + T01 * R(1, 2) + T02 * R(2, 2));
}

////////////////////////////////////////////////////////////////////////

void
Element::polarDecompositionEigen(const ColumnMajorMatrix & Fhat,
                                 ColumnMajorMatrix & Rhat,
                                 SymmTensor & strain_increment)
{
  const int ND = 3;

  ColumnMajorMatrix eigen_value(ND, 1), eigen_vector(ND, ND);
  ColumnMajorMatrix n1(ND, 1), n2(ND, 1), n3(ND, 1), N1(ND, 1), N2(ND, 1), N3(ND, 1);

  ColumnMajorMatrix Chat = Fhat.transpose() * Fhat;

  Chat.eigen(eigen_value, eigen_vector);

  for (int i = 0; i < ND; i++)
  {
    N1(i) = eigen_vector(i, 0);
    N2(i) = eigen_vector(i, 1);
    N3(i) = eigen_vector(i, 2);
  }

  const Real lamda1 = std::sqrt(eigen_value(0));
  const Real lamda2 = std::sqrt(eigen_value(1));
  const Real lamda3 = std::sqrt(eigen_value(2));

  const Real log1 = std::log(lamda1);
  const Real log2 = std::log(lamda2);
  const Real log3 = std::log(lamda3);

  ColumnMajorMatrix Uhat =
      N1 * N1.transpose() * lamda1 + N2 * N2.transpose() * lamda2 + N3 * N3.transpose() * lamda3;

  ColumnMajorMatrix invUhat(ND, ND);
  invertMatrix(Uhat, invUhat);

  Rhat = Fhat * invUhat;

  strain_increment =
      N1 * N1.transpose() * log1 + N2 * N2.transpose() * log2 + N3 * N3.transpose() * log3;
}

////////////////////////////////////////////////////////////////////////

void
Element::fillMatrix(unsigned int qp,
                    const VariableGradient & grad_x,
                    const VariableGradient & grad_y,
                    const VariableGradient & grad_z,
                    ColumnMajorMatrix & A)
{
  A(0, 0) = grad_x[qp](0);
  A(0, 1) = grad_x[qp](1);
  A(0, 2) = grad_x[qp](2);
  A(1, 0) = grad_y[qp](0);
  A(1, 1) = grad_y[qp](1);
  A(1, 2) = grad_y[qp](2);
  A(2, 0) = grad_z[qp](0);
  A(2, 1) = grad_z[qp](1);
  A(2, 2) = grad_z[qp](2);
}

////////////////////////////////////////////////////////////////////////

} // namespace solid_mechanics
