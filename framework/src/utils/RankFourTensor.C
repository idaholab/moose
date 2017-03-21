/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RankFourTensor.h"
#include "RankTwoTensor.h"
#include "MooseException.h"
#include "MatrixTools.h"
#include "MaterialProperty.h"

// Any other includes here
#include "libmesh/utility.h"
#include <ostream>

template <>
void
mooseSetToZero<RankFourTensor>(RankFourTensor & v)
{
  v.zero();
}

template <>
void
dataStore(std::ostream & stream, RankFourTensor & rft, void * context)
{
  dataStore(stream, rft._vals, context);
}

template <>
void
dataLoad(std::istream & stream, RankFourTensor & rft, void * context)
{
  dataLoad(stream, rft._vals, context);
}

MooseEnum
RankFourTensor::fillMethodEnum()
{
  return MooseEnum("antisymmetric symmetric9 symmetric21 general_isotropic symmetric_isotropic "
                   "antisymmetric_isotropic axisymmetric_rz general principal");
}

RankFourTensor::RankFourTensor()
{
  mooseAssert(N == 3, "RankFourTensor is currently only tested for 3 dimensions.");

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensor::RankFourTensor(const InitMethod init)
{
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              _vals[i][j][k][l] = 0.0;
      for (unsigned int i = 0; i < N; ++i)
        _vals[i][i][i][i] = 1.0;
      break;

    case initIdentityFour:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              _vals[i][j][k][l] = (i == k) && (j == l);
      break;

    case initIdentitySymmetricFour:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              _vals[i][j][k][l] = 0.5 * ((i == k) && (j == l)) + 0.5 * ((i == l) && (j == k));
      break;

    default:
      mooseError("Unknown RankFourTensor initialization pattern.");
  }
}

RankFourTensor::RankFourTensor(const std::vector<Real> & input, FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

Real &
RankFourTensor::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  return _vals[i][j][k][l];
}

Real
RankFourTensor::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  return _vals[i][j][k][l];
}

void
RankFourTensor::zero()
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensor &
RankFourTensor::operator=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] = a(i, j, k, l);

  return *this;
}

RankTwoTensor RankFourTensor::operator*(const RankTwoTensor & b) const
{
  RealTensorValue result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j) += a(i, j, k, l) * b(k, l);

  return result;
}

RealTensorValue RankFourTensor::operator*(const RealTensorValue & b) const
{
  RealTensorValue result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j) += a(i, j, k, l) * b(k, l);

  return result;
}

RankFourTensor RankFourTensor::operator*(const Real b) const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = a(i, j, k, l) * b;

  return result;
}

RankFourTensor &
RankFourTensor::operator*=(const Real a)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] *= a;

  return *this;
}

RankFourTensor
RankFourTensor::operator/(const Real b) const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = a(i, j, k, l) / b;

  return result;
}

RankFourTensor &
RankFourTensor::operator/=(const Real a)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] /= a;

  return *this;
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] += a(i, j, k, l);

  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor & b) const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = a(i, j, k, l) + b(i, j, k, l);

  return result;
}

RankFourTensor &
RankFourTensor::operator-=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][k][l] -= a(i, j, k, l);

  return *this;
}

RankFourTensor
RankFourTensor::operator-(const RankFourTensor & b) const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = a(i, j, k, l) - b(i, j, k, l);

  return result;
}

RankFourTensor
RankFourTensor::operator-() const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = -a(i, j, k, l);

  return result;
}

RankFourTensor RankFourTensor::operator*(const RankFourTensor & b) const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          for (unsigned int p = 0; p < N; ++p)
            for (unsigned int q = 0; q < N; ++q)
              result(i, j, k, l) += a(i, j, p, q) * b(p, q, k, l);

  return result;
}

Real
RankFourTensor::L2norm() const
{
  Real l2 = 0;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          l2 += Utility::pow<2>(a(i, j, k, l));

  return std::sqrt(l2);
}

RankFourTensor
RankFourTensor::invSymm() const
{
  unsigned int ntens = N * (N + 1) / 2;
  int nskip = N - 1;

  RankFourTensor result;
  const RankFourTensor & a = *this;
  std::vector<PetscScalar> mat;
  mat.assign(ntens * ntens, 0);

  // We use the LAPACK matrix inversion routine here.  Form the matrix
  //
  // mat[0]  mat[1]  mat[2]  mat[3]  mat[4]  mat[5]
  // mat[6]  mat[7]  mat[8]  mat[9]  mat[10] mat[11]
  // mat[12] mat[13] mat[14] mat[15] mat[16] mat[17]
  // mat[18] mat[19] mat[20] mat[21] mat[22] mat[23]
  // mat[24] mat[25] mat[26] mat[27] mat[28] mat[29]
  // mat[30] mat[31] mat[32] mat[33] mat[34] mat[35]
  //
  // This is filled from the indpendent components of C assuming
  // the symmetry C_ijkl = C_ijlk = C_jikl.
  //
  // If there are two rank-four tensors X and Y then the reason for
  // this filling becomes apparent if we want to calculate
  // X_ijkl*Y_klmn = Z_ijmn
  // For denote the "mat" versions of X, Y and Z by x, y and z.
  // Then
  // z_ab = x_ac*y_cb
  // Eg
  // z_00 = Z_0000 = X_0000*Y_0000 + X_0011*Y_1111 + X_0022*Y_2200 + 2*X_0001*Y_0100 +
  // 2*X_0002*Y_0200 + 2*X_0012*Y_1200   (the factors of 2 come from the assumed symmetries)
  // z_03 = 2*Z_0001 = X_0000*2*Y_0001 + X_0011*2*Y_1101 + X_0022*2*Y_2201 + 2*X_0001*2*Y_0101 +
  // 2*X_0002*2*Y_0201 + 2*X_0012*2*Y_1201
  // z_22 = 2*Z_0102 = X_0100*2*Y_0002 + X_0111*2*X_1102 + X_0122*2*Y_2202 + 2*X_0101*2*Y_0102 +
  // 2*X_0102*2*Y_0202 + 2*X_0112*2*Y_1202
  // Finally, we use LAPACK to find x^-1, and put it back into rank-4 tensor form
  //
  // mat[0] = C(0,0,0,0)
  // mat[1] = C(0,0,1,1)
  // mat[2] = C(0,0,2,2)
  // mat[3] = C(0,0,0,1)*2
  // mat[4] = C(0,0,0,2)*2
  // mat[5] = C(0,0,1,2)*2

  // mat[6] = C(1,1,0,0)
  // mat[7] = C(1,1,1,1)
  // mat[8] = C(1,1,2,2)
  // mat[9] = C(1,1,0,1)*2
  // mat[10] = C(1,1,0,2)*2
  // mat[11] = C(1,1,1,2)*2

  // mat[12] = C(2,2,0,0)
  // mat[13] = C(2,2,1,1)
  // mat[14] = C(2,2,2,2)
  // mat[15] = C(2,2,0,1)*2
  // mat[16] = C(2,2,0,2)*2
  // mat[17] = C(2,2,1,2)*2

  // mat[18] = C(0,1,0,0)
  // mat[19] = C(0,1,1,1)
  // mat[20] = C(0,1,2,2)
  // mat[21] = C(0,1,0,1)*2
  // mat[22] = C(0,1,0,2)*2
  // mat[23] = C(0,1,1,2)*2

  // mat[24] = C(0,2,0,0)
  // mat[25] = C(0,2,1,1)
  // mat[26] = C(0,2,2,2)
  // mat[27] = C(0,2,0,1)*2
  // mat[28] = C(0,2,0,2)*2
  // mat[29] = C(0,2,1,2)*2

  // mat[30] = C(1,2,0,0)
  // mat[31] = C(1,2,1,1)
  // mat[32] = C(1,2,2,2)
  // mat[33] = C(1,2,0,1)*2
  // mat[34] = C(1,2,0,2)*2
  // mat[35] = C(1,2,1,2)*2

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i == j)
            mat[k == l ? i * ntens + k : i * ntens + k + nskip + l] += a(i, j, k, l);
          else // i!=j
            mat[k == l ? (nskip + i + j) * ntens + k : (nskip + i + j) * ntens + k + nskip + l] +=
                a(i,
                  j,
                  k,
                  l); // note the +=, which results in double-counting and is rectified below
        }

  for (unsigned int i = 3; i < ntens; ++i)
    for (unsigned int j = 0; j < ntens; ++j)
      mat[i * ntens + j] /= 2.0; // because of double-counting above

  // use LAPACK to find the inverse
  MatrixTools::inverse(mat, ntens);

  // build the resulting rank-four tensor
  // using the inverse of the above algorithm
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i == j)
            result(i, j, k, l) = k == l ? mat[i * ntens + k] : mat[i * ntens + k + nskip + l] / 2.0;
          else // i!=j
            result(i, j, k, l) = k == l ? mat[(nskip + i + j) * ntens + k]
                                        : mat[(nskip + i + j) * ntens + k + nskip + l] / 2.0;
        }

  return result;
}

void
RankFourTensor::rotate(const RealTensorValue & R)
{
  RankFourTensor old = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          Real sum = 0.0;
          for (unsigned int m = 0; m < N; ++m)
            for (unsigned int n = 0; n < N; ++n)
              for (unsigned int o = 0; o < N; ++o)
                for (unsigned int p = 0; p < N; ++p)
                  sum += R(i, m) * R(j, n) * R(k, o) * R(l, p) * old(m, n, o, p);

          _vals[i][j][k][l] = sum;
        }
}

void
RankFourTensor::rotate(const RankTwoTensor & R)
{
  RankFourTensor old = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          Real sum = 0.0;
          for (unsigned int m = 0; m < N; ++m)
            for (unsigned int n = 0; n < N; ++n)
              for (unsigned int o = 0; o < N; ++o)
                for (unsigned int p = 0; p < N; ++p)
                  sum += R(i, m) * R(j, n) * R(k, o) * R(l, p) * old(m, n, o, p);

          _vals[i][j][k][l] = sum;
        }
}

void
RankFourTensor::print(std::ostream & stm) const
{
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      stm << "i = " << i << " j = " << j << '\n';
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
          stm << std::setw(15) << a(i, j, k, l) << " ";

        stm << '\n';
      }
    }
}

RankFourTensor
RankFourTensor::transposeMajor() const
{
  RankFourTensor result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j, k, l) = a(k, l, i, j);

  return result;
}

void
RankFourTensor::surfaceFillFromInputVector(const std::vector<Real> & input)
{
  zero();

  if (input.size() == 9)
  {
    // then fill from vector C_1111, C_1112, C_1122, C_1212, C_1222, C_1211, C_2211, C_2212, C_2222
    _vals[0][0][0][0] = input[0];
    _vals[0][0][0][1] = input[1];
    _vals[0][0][1][1] = input[2];
    _vals[0][1][0][1] = input[3];
    _vals[0][1][1][1] = input[4];
    _vals[0][1][0][0] = input[5];
    _vals[1][1][0][0] = input[6];
    _vals[1][1][0][1] = input[7];
    _vals[1][1][1][1] = input[8];

    // fill in remainders from C_ijkl = C_ijlk = C_jikl
    _vals[0][0][1][0] = _vals[0][0][0][1];
    _vals[0][1][1][0] = _vals[0][1][0][1];
    _vals[1][0][0][0] = _vals[0][1][0][0];
    _vals[1][0][0][1] = _vals[0][1][0][1];
    _vals[1][0][1][1] = _vals[0][1][1][1];
    _vals[1][0][0][0] = _vals[0][1][0][0];
    _vals[1][1][1][0] = _vals[1][1][0][1];
  }
  else if (input.size() == 2)
  {
    // only two independent constants, C_1111 and C_1122
    _vals[0][0][0][0] = input[0];
    _vals[0][0][1][1] = input[1];
    // use symmetries
    _vals[1][1][1][1] = _vals[0][0][0][0];
    _vals[1][1][0][0] = _vals[0][0][1][1];
    _vals[0][1][0][1] = 0.5 * (_vals[0][0][0][0] - _vals[0][0][1][1]);
    _vals[1][0][0][1] = _vals[0][1][0][1];
    _vals[0][1][1][0] = _vals[0][1][0][1];
    _vals[1][0][1][0] = _vals[0][1][0][1];
  }
  else
    mooseError(
        "Please provide correct number of inputs for surface RankFourTensor initialization.");
}

void
RankFourTensor::fillFromInputVector(const std::vector<Real> & input, FillMethod fill_method)
{
  zero();

  switch (fill_method)
  {
    case antisymmetric:
      fillAntisymmetricFromInputVector(input);
      break;
    case symmetric9:
      fillSymmetricFromInputVector(input, false);
      break;
    case symmetric21:
      fillSymmetricFromInputVector(input, true);
      break;
    case general_isotropic:
      fillGeneralIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic:
      fillSymmetricIsotropicFromInputVector(input);
      break;
    case antisymmetric_isotropic:
      fillAntisymmetricIsotropicFromInputVector(input);
      break;
    case axisymmetric_rz:
      fillAxisymmetricRZFromInputVector(input);
      break;
    case general:
      fillGeneralFromInputVector(input);
      break;
    case principal:
      fillPrincipalFromInputVector(input);
      break;
    default:
      mooseError("fillFromInputVector called with unknown fill_method of ", fill_method);
  }
}

void
RankFourTensor::fillSymmetricFromInputVector(const std::vector<Real> & input, bool all)
{
  if ((all == true && input.size() != 21) || (all == false && input.size() != 9))
    mooseError("Please check the number of entries in the stiffness input vector.");

  zero();

  if (all == true)
  {
    _vals[0][0][0][0] = input[0]; // C1111
    _vals[0][0][1][1] = input[1]; // C1122
    _vals[0][0][2][2] = input[2]; // C1133
    _vals[0][0][1][2] = input[3]; // C1123
    _vals[0][0][0][2] = input[4]; // C1113
    _vals[0][0][0][1] = input[5]; // C1112

    _vals[1][1][1][1] = input[6];  // C2222
    _vals[1][1][2][2] = input[7];  // C2233
    _vals[1][1][1][2] = input[8];  // C2223
    _vals[0][2][1][1] = input[9];  // C2213  //flipped for filling purposes
    _vals[0][1][1][1] = input[10]; // C2212 //flipped for filling purposes

    _vals[2][2][2][2] = input[11]; // C3333
    _vals[1][2][2][2] = input[12]; // C3323 //flipped for filling purposes
    _vals[0][2][2][2] = input[13]; // C3313 //flipped for filling purposes
    _vals[0][1][2][2] = input[14]; // C3312 //flipped for filling purposes

    _vals[1][2][1][2] = input[15]; // C2323
    _vals[0][2][1][2] = input[16]; // C2313 //flipped for filling purposes
    _vals[0][1][1][2] = input[17]; // C2312 //flipped for filling purposes

    _vals[0][2][0][2] = input[18]; // C1313
    _vals[0][1][0][2] = input[19]; // C1312 //flipped for filling purposes

    _vals[0][1][0][1] = input[20]; // C1212
  }
  else
  {
    _vals[0][0][0][0] = input[0]; // C1111
    _vals[0][0][1][1] = input[1]; // C1122
    _vals[0][0][2][2] = input[2]; // C1133
    _vals[1][1][1][1] = input[3]; // C2222
    _vals[1][1][2][2] = input[4]; // C2233
    _vals[2][2][2][2] = input[5]; // C3333
    _vals[1][2][1][2] = input[6]; // C2323
    _vals[0][2][0][2] = input[7]; // C1313
    _vals[0][1][0][1] = input[8]; // C1212
  }

  // fill in from symmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          _vals[i][j][l][k] = _vals[j][i][k][l] = _vals[j][i][l][k] = _vals[k][l][i][j] =
              _vals[l][k][j][i] = _vals[k][l][j][i] = _vals[l][k][i][j] = _vals[i][j][k][l];
}

void
RankFourTensor::fillAntisymmetricFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 6)
    mooseError(
        "To use fillAntisymmetricFromInputVector, your input must have size 6.  Yours has size ",
        input.size());

  zero();

  _vals[0][1][0][1] = input[0]; // B1212
  _vals[0][1][0][2] = input[1]; // B1213
  _vals[0][1][1][2] = input[2]; // B1223

  _vals[0][2][0][2] = input[3]; // B1313
  _vals[0][2][1][2] = input[4]; // B1323

  _vals[1][2][1][2] = input[5]; // B2323

  // symmetry on the two pairs
  _vals[0][2][0][1] = _vals[0][1][0][2];
  _vals[1][2][0][1] = _vals[0][1][1][2];
  _vals[1][2][0][2] = _vals[0][2][1][2];
  // have now got the upper parts of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      _vals[0][1][j][i] = -_vals[0][1][i][j];
      _vals[0][2][j][i] = -_vals[0][2][i][j];
      _vals[1][2][j][i] = -_vals[1][2][i][j];
    }
  // have now got all of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      _vals[1][0][i][j] = -_vals[0][1][i][j];
      _vals[2][0][i][j] = -_vals[0][2][i][j];
      _vals[2][1][i][j] = -_vals[1][2][i][j];
    }
}

void
RankFourTensor::fillGeneralIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 3)
    mooseError(
        "To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours has size ",
        input.size());

  zero();

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          _vals[i][j][k][l] = input[0] * (i == j) * (k == l) + input[1] * (i == k) * (j == l) +
                              input[1] * (i == l) * (j == k);
          for (unsigned int m = 0; m < N; ++m)
            _vals[i][j][k][l] +=
                input[2] * PermutationTensor::eps(i, j, m) * PermutationTensor::eps(k, l, m);
        }
}

void
RankFourTensor::fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1. "
               "Yours has size ",
               input.size());
  fillGeneralIsotropicFromInputVector({0.0, 0.0, input[0]});
}

void
RankFourTensor::fillSymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 2)
    mooseError("To use fillSymmetricIsotropicFromInputVector, your input must have size 2. Yours "
               "has size ",
               input.size());
  fillGeneralIsotropicFromInputVector({input[0], input[1], 0.0});
}

void
RankFourTensor::fillAxisymmetricRZFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 5)
    mooseError("To use fillAxisymmetricRZFromInputVector, your input must have size 5.  Your "
               "vector has size ",
               input.size());

  // C1111     C1122     C1133     C2222     C2233=C1133
  fillSymmetricFromInputVector({input[0],
                                input[1],
                                input[2],
                                input[0],
                                input[2],
                                // C3333     C2323     C3131=C2323   C1212
                                input[3],
                                input[4],
                                input[4],
                                (input[0] - input[1]) * 0.5},
                               false);
}

void
RankFourTensor::fillGeneralFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 81)
    mooseError("To use fillGeneralFromInputVector, your input must have size 81. Yours has size ",
               input.size());

  int ind;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          ind = i * N * N * N + j * N * N + k * N + l;
          _vals[i][j][k][l] = input[ind];
        }
}

void
RankFourTensor::fillPrincipalFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 9)
    mooseError("To use fillPrincipalFromInputVector, your input must have size 9. Yours has size ",
               input.size());

  zero();

  _vals[0][0][0][0] = input[0];
  _vals[0][0][1][1] = input[1];
  _vals[0][0][2][2] = input[2];
  _vals[1][1][0][0] = input[3];
  _vals[1][1][1][1] = input[4];
  _vals[1][1][2][2] = input[5];
  _vals[2][2][0][0] = input[6];
  _vals[2][2][1][1] = input[7];
  _vals[2][2][2][2] = input[8];
}

RankTwoTensor
RankFourTensor::innerProductTranspose(const RankTwoTensor & b) const
{
  RealTensorValue result;
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(k, l) += a(i, j, k, l) * b(i, j);

  return result;
}

Real
RankFourTensor::sum3x3() const
{
  // summation of Ciijj for i and j ranging from 0 to 2 - used in the volumetric locking correction
  return _vals[0][0][0][0] + _vals[0][0][1][1] + _vals[0][0][2][2] + _vals[1][1][0][0] +
         _vals[1][1][1][1] + _vals[1][1][2][2] + _vals[2][2][0][0] + _vals[2][2][1][1] +
         _vals[2][2][2][2];
}

RealGradient
RankFourTensor::sum3x1() const
{
  // used for volumetric locking correction
  RealGradient a(3);
  a(0) = _vals[0][0][0][0] + _vals[0][0][1][1] + _vals[0][0][2][2]; // C0000 + C0011 + C0022
  a(1) = _vals[1][1][0][0] + _vals[1][1][1][1] + _vals[1][1][2][2]; // C1100 + C1111 + C1122
  a(2) = _vals[2][2][0][0] + _vals[2][2][1][1] + _vals[2][2][2][2]; // C2200 + C2211 + C2222
  return a;
}
