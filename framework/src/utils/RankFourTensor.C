//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankFourTensor.h"

// MOOSE includes
#include "RankTwoTensor.h"
#include "MooseEnum.h"
#include "MooseException.h"
#include "MooseUtils.h"
#include "MatrixTools.h"
#include "MaterialProperty.h"
#include "PermutationTensor.h"

#include "libmesh/utility.h"

// C++ includes
#include <iomanip>
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
                   "symmetric_isotropic_E_nu antisymmetric_isotropic axisymmetric_rz general "
                   "principal");
}

RankFourTensor::RankFourTensor()
{
  mooseAssert(N == 3, "RankFourTensor is currently only tested for 3 dimensions.");

  unsigned int index = 0;
  for (unsigned int i = 0; i < N4; ++i)
    _vals[index++] = 0.0;
}

RankFourTensor::RankFourTensor(const InitMethod init)
{
  unsigned int index = 0;
  switch (init)
  {
    case initNone:
      break;

    case initIdentity:
      zero();
      for (unsigned int i = 0; i < N; ++i)
        (*this)(i, i, i, i) = 1.0;
      break;

    case initIdentityFour:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              _vals[index++] = (i == k) && (j == l);
      break;

    case initIdentitySymmetricFour:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              _vals[index++] = 0.5 * ((i == k) && (j == l)) + 0.5 * ((i == l) && (j == k));
      break;

    default:
      mooseError("Unknown RankFourTensor initialization pattern.");
  }
}

RankFourTensor::RankFourTensor(const std::vector<Real> & input, FillMethod fill_method)
{
  fillFromInputVector(input, fill_method);
}

void
RankFourTensor::zero()
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] = 0.0;
}

RankFourTensor &
RankFourTensor::operator=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] = a._vals[i];
  return *this;
}

RankTwoTensor RankFourTensor::operator*(const RankTwoTensor & b) const
{
  RankTwoTensor result;

  unsigned int index = 0;
  for (unsigned int ij = 0; ij < N2; ++ij)
  {
    Real tmp = 0;
    for (unsigned int kl = 0; kl < N2; ++kl)
      tmp += _vals[index++] * b._vals[kl];
    result._vals[ij] = tmp;
  }

  return result;
}

RealTensorValue RankFourTensor::operator*(const RealTensorValue & b) const
{
  RealTensorValue result;

  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i, j) += _vals[index++] * b(k, l);

  return result;
}

RankFourTensor RankFourTensor::operator*(const Real b) const
{
  RankFourTensor result;

  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] * b;

  return result;
}

RankFourTensor &
RankFourTensor::operator*=(const Real a)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] *= a;
  return *this;
}

RankFourTensor
RankFourTensor::operator/(const Real b) const
{
  RankFourTensor result;
  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] / b;
  return result;
}

RankFourTensor &
RankFourTensor::operator/=(const Real a)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] /= a;
  return *this;
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] += a._vals[i];
  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor & b) const
{
  RankFourTensor result;
  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] + b._vals[i];
  return result;
}

RankFourTensor &
RankFourTensor::operator-=(const RankFourTensor & a)
{
  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] -= a._vals[i];
  return *this;
}

RankFourTensor
RankFourTensor::operator-(const RankFourTensor & b) const
{
  RankFourTensor result;
  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = _vals[i] - b._vals[i];
  return result;
}

RankFourTensor
RankFourTensor::operator-() const
{
  RankFourTensor result;
  for (unsigned int i = 0; i < N4; ++i)
    result._vals[i] = -_vals[i];
  return result;
}

RankFourTensor RankFourTensor::operator*(const RankFourTensor & b) const
{
  RankFourTensor result;

  unsigned int index = 0;
  unsigned int ij1 = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    for (unsigned int j = 0; j < N; ++j)
    {
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
        {
          Real sum = 0;
          for (unsigned int p = 0; p < N; ++p)
          {
            unsigned int p1 = N * p;
            for (unsigned int q = 0; q < N; ++q)
              sum += _vals[ij1 + p1 + q] * b(p, q, k, l);
          }
          result._vals[index++] = sum;
        }
      }
      ij1 += N2;
    }
  }

  return result;
}

Real
RankFourTensor::L2norm() const
{
  Real l2 = 0;

  for (unsigned int i = 0; i < N4; ++i)
    l2 += Utility::pow<2>(_vals[i]);

  return std::sqrt(l2);
}

RankFourTensor
RankFourTensor::invSymm() const
{
  unsigned int ntens = N * (N + 1) / 2;
  int nskip = N - 1;

  RankFourTensor result;
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

  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i == j)
            mat[k == l ? i * ntens + k : i * ntens + k + nskip + l] += _vals[index];
          else // i!=j
            mat[k == l ? (nskip + i + j) * ntens + k : (nskip + i + j) * ntens + k + nskip + l] +=
                _vals[index]; // note the +=, which results in double-counting and is rectified
                              // below
          index++;
        }

  for (unsigned int i = 3; i < ntens; ++i)
    for (unsigned int j = 0; j < ntens; ++j)
      mat[i * ntens + j] /= 2.0; // because of double-counting above

  // use LAPACK to find the inverse
  MatrixTools::inverse(mat, ntens);

  // build the resulting rank-four tensor
  // using the inverse of the above algorithm
  index = 0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i == j)
            result._vals[index] =
                k == l ? mat[i * ntens + k] : mat[i * ntens + k + nskip + l] / 2.0;
          else // i!=j
            result._vals[index] = k == l ? mat[(nskip + i + j) * ntens + k]
                                         : mat[(nskip + i + j) * ntens + k + nskip + l] / 2.0;
          index++;
        }

  return result;
}

void
RankFourTensor::rotate(const RealTensorValue & R)
{
  RankFourTensor old = *this;

  unsigned int index = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    for (unsigned int j = 0; j < N; ++j)
    {
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
        {
          unsigned int index2 = 0;
          Real sum = 0.0;
          for (unsigned int m = 0; m < N; ++m)
          {
            const Real a = R(i, m);
            for (unsigned int n = 0; n < N; ++n)
            {
              const Real ab = a * R(j, n);
              for (unsigned int o = 0; o < N; ++o)
              {
                const Real abc = ab * R(k, o);
                for (unsigned int p = 0; p < N; ++p)
                  sum += abc * R(l, p) * old._vals[index2++];
              }
            }
          }
          _vals[index++] = sum;
        }
      }
    }
  }
}

void
RankFourTensor::rotate(const RankTwoTensor & R)
{
  RankFourTensor old = *this;

  unsigned int index = 0;
  unsigned int i1 = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    unsigned int j1 = 0;
    for (unsigned int j = 0; j < N; ++j)
    {
      unsigned int k1 = 0;
      for (unsigned int k = 0; k < N; ++k)
      {
        unsigned int l1 = 0;
        for (unsigned int l = 0; l < N; ++l)
        {
          unsigned int index2 = 0;
          Real sum = 0.0;
          for (unsigned int m = 0; m < N; ++m)
          {
            const Real a = R._vals[i1 + m];
            for (unsigned int n = 0; n < N; ++n)
            {
              const Real ab = a * R._vals[j1 + n];
              for (unsigned int o = 0; o < N; ++o)
              {
                const Real abc = ab * R._vals[k1 + o];
                for (unsigned int p = 0; p < N; ++p)
                  sum += abc * R._vals[l1 + p] * old._vals[index2++];
              }
            }
          }
          _vals[index++] = sum;
          l1 += N;
        }
        k1 += N;
      }
      j1 += N;
    }
    i1 += N;
  }
}

void
RankFourTensor::print(std::ostream & stm) const
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      stm << "i = " << i << " j = " << j << '\n';
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
          stm << std::setw(15) << (*this)(i, j, k, l) << " ";

        stm << '\n';
      }
    }
}

RankFourTensor
RankFourTensor::transposeMajor() const
{
  RankFourTensor result;

  unsigned int index = 0;
  unsigned int i1 = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    for (unsigned int j = 0; j < N; ++j)
    {
      for (unsigned int k = 0; k < N; ++k)
      {
        unsigned int ijk1 = k * N3 + i1 + j;
        for (unsigned int l = 0; l < N; ++l)
          result._vals[index++] = _vals[ijk1 + l * N2];
      }
    }
    i1 += N;
  }

  return result;
}

void
RankFourTensor::surfaceFillFromInputVector(const std::vector<Real> & input)
{
  zero();

  if (input.size() == 9)
  {
    // then fill from vector C_1111, C_1112, C_1122, C_1212, C_1222, C_1211, C_2211, C_2212, C_2222
    (*this)(0, 0, 0, 0) = input[0];
    (*this)(0, 0, 0, 1) = input[1];
    (*this)(0, 0, 1, 1) = input[2];
    (*this)(0, 1, 0, 1) = input[3];
    (*this)(0, 1, 1, 1) = input[4];
    (*this)(0, 1, 0, 0) = input[5];
    (*this)(1, 1, 0, 0) = input[6];
    (*this)(1, 1, 0, 1) = input[7];
    (*this)(1, 1, 1, 1) = input[8];

    // fill in remainders from C_ijkl = C_ijlk = C_jikl
    (*this)(0, 0, 1, 0) = (*this)(0, 0, 0, 1);
    (*this)(0, 1, 1, 0) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 0, 0) = (*this)(0, 1, 0, 0);
    (*this)(1, 0, 0, 1) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 1, 1) = (*this)(0, 1, 1, 1);
    (*this)(1, 0, 0, 0) = (*this)(0, 1, 0, 0);
    (*this)(1, 1, 1, 0) = (*this)(1, 1, 0, 1);
  }
  else if (input.size() == 2)
  {
    // only two independent constants, C_1111 and C_1122
    (*this)(0, 0, 0, 0) = input[0];
    (*this)(0, 0, 1, 1) = input[1];
    // use symmetries
    (*this)(1, 1, 1, 1) = (*this)(0, 0, 0, 0);
    (*this)(1, 1, 0, 0) = (*this)(0, 0, 1, 1);
    (*this)(0, 1, 0, 1) = 0.5 * ((*this)(0, 0, 0, 0) - (*this)(0, 0, 1, 1));
    (*this)(1, 0, 0, 1) = (*this)(0, 1, 0, 1);
    (*this)(0, 1, 1, 0) = (*this)(0, 1, 0, 1);
    (*this)(1, 0, 1, 0) = (*this)(0, 1, 0, 1);
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
    case symmetric_isotropic_E_nu:
      fillSymmetricIsotropicEandNuFromInputVector(input);
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
    (*this)(0, 0, 0, 0) = input[0]; // C1111
    (*this)(0, 0, 1, 1) = input[1]; // C1122
    (*this)(0, 0, 2, 2) = input[2]; // C1133
    (*this)(0, 0, 1, 2) = input[3]; // C1123
    (*this)(0, 0, 0, 2) = input[4]; // C1113
    (*this)(0, 0, 0, 1) = input[5]; // C1112

    (*this)(1, 1, 1, 1) = input[6];  // C2222
    (*this)(1, 1, 2, 2) = input[7];  // C2233
    (*this)(1, 1, 1, 2) = input[8];  // C2223
    (*this)(0, 2, 1, 1) = input[9];  // C2213  //flipped for filling purposes
    (*this)(0, 1, 1, 1) = input[10]; // C2212 //flipped for filling purposes

    (*this)(2, 2, 2, 2) = input[11]; // C3333
    (*this)(1, 2, 2, 2) = input[12]; // C3323 //flipped for filling purposes
    (*this)(0, 2, 2, 2) = input[13]; // C3313 //flipped for filling purposes
    (*this)(0, 1, 2, 2) = input[14]; // C3312 //flipped for filling purposes

    (*this)(1, 2, 1, 2) = input[15]; // C2323
    (*this)(0, 2, 1, 2) = input[16]; // C2313 //flipped for filling purposes
    (*this)(0, 1, 1, 2) = input[17]; // C2312 //flipped for filling purposes

    (*this)(0, 2, 0, 2) = input[18]; // C1313
    (*this)(0, 1, 0, 2) = input[19]; // C1312 //flipped for filling purposes

    (*this)(0, 1, 0, 1) = input[20]; // C1212
  }
  else
  {
    (*this)(0, 0, 0, 0) = input[0]; // C1111
    (*this)(0, 0, 1, 1) = input[1]; // C1122
    (*this)(0, 0, 2, 2) = input[2]; // C1133
    (*this)(1, 1, 1, 1) = input[3]; // C2222
    (*this)(1, 1, 2, 2) = input[4]; // C2233
    (*this)(2, 2, 2, 2) = input[5]; // C3333
    (*this)(1, 2, 1, 2) = input[6]; // C2323
    (*this)(0, 2, 0, 2) = input[7]; // C1313
    (*this)(0, 1, 0, 1) = input[8]; // C1212
  }

  // fill in from symmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          (*this)(i, j, l, k) = (*this)(j, i, k, l) = (*this)(j, i, l, k) = (*this)(k, l, i, j) =
              (*this)(l, k, j, i) = (*this)(k, l, j, i) = (*this)(l, k, i, j) = (*this)(i, j, k, l);
}

void
RankFourTensor::fillAntisymmetricFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 6)
    mooseError(
        "To use fillAntisymmetricFromInputVector, your input must have size 6.  Yours has size ",
        input.size());

  zero();

  (*this)(0, 1, 0, 1) = input[0]; // B1212
  (*this)(0, 1, 0, 2) = input[1]; // B1213
  (*this)(0, 1, 1, 2) = input[2]; // B1223

  (*this)(0, 2, 0, 2) = input[3]; // B1313
  (*this)(0, 2, 1, 2) = input[4]; // B1323

  (*this)(1, 2, 1, 2) = input[5]; // B2323

  // symmetry on the two pairs
  (*this)(0, 2, 0, 1) = (*this)(0, 1, 0, 2);
  (*this)(1, 2, 0, 1) = (*this)(0, 1, 1, 2);
  (*this)(1, 2, 0, 2) = (*this)(0, 2, 1, 2);
  // have now got the upper parts of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      (*this)(0, 1, j, i) = -(*this)(0, 1, i, j);
      (*this)(0, 2, j, i) = -(*this)(0, 2, i, j);
      (*this)(1, 2, j, i) = -(*this)(1, 2, i, j);
    }
  // have now got all of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      (*this)(1, 0, i, j) = -(*this)(0, 1, i, j);
      (*this)(2, 0, i, j) = -(*this)(0, 2, i, j);
      (*this)(2, 1, i, j) = -(*this)(1, 2, i, j);
    }
}

void
RankFourTensor::fillGeneralIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 3)
    mooseError(
        "To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours has size ",
        input.size());

  fillGeneralIsotropic(input[0], input[1], input[2]);
}

void
RankFourTensor::fillGeneralIsotropic(Real i0, Real i1, Real i2)
{
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          (*this)(i, j, k, l) =
              i0 * (i == j) * (k == l) + i1 * (i == k) * (j == l) + i1 * (i == l) * (j == k);
          for (unsigned int m = 0; m < N; ++m)
            (*this)(i, j, k, l) +=
                i2 * PermutationTensor::eps(i, j, m) * PermutationTensor::eps(k, l, m);
        }
}

void
RankFourTensor::fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1. "
               "Yours has size ",
               input.size());
  fillGeneralIsotropic(0.0, 0.0, input[0]);
}

void
RankFourTensor::fillAntisymmetricIsotropic(Real i0)
{
  fillGeneralIsotropic(0.0, 0.0, i0);
}

void
RankFourTensor::fillSymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 2)
    mooseError("To use fillSymmetricIsotropicFromInputVector, your input must have size 2. Yours "
               "has size ",
               input.size());
  fillGeneralIsotropic(input[0], input[1], 0.0);
}

void
RankFourTensor::fillSymmetricIsotropic(Real i0, Real i1)
{
  fillGeneralIsotropic(i0, i1, 0.0);
}

void
RankFourTensor::fillSymmetricIsotropicEandNuFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 2)
    mooseError(
        "To use fillSymmetricIsotropicEandNuFromInputVector, your input must have size 2. Yours "
        "has size ",
        input.size());

  fillSymmetricIsotropicEandNu(input[0], input[1]);
}

void
RankFourTensor::fillSymmetricIsotropicEandNu(Real E, Real nu)
{
  // Calculate lambda and the shear modulus from the given young's modulus and poisson's ratio
  const Real lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
  const Real G = E / (2.0 * (1.0 + nu));

  fillGeneralIsotropic(lambda, G, 0.0);
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

  for (unsigned int i = 0; i < N4; ++i)
    _vals[i] = input[i];
}

void
RankFourTensor::fillPrincipalFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 9)
    mooseError("To use fillPrincipalFromInputVector, your input must have size 9. Yours has size ",
               input.size());

  zero();

  (*this)(0, 0, 0, 0) = input[0];
  (*this)(0, 0, 1, 1) = input[1];
  (*this)(0, 0, 2, 2) = input[2];
  (*this)(1, 1, 0, 0) = input[3];
  (*this)(1, 1, 1, 1) = input[4];
  (*this)(1, 1, 2, 2) = input[5];
  (*this)(2, 2, 0, 0) = input[6];
  (*this)(2, 2, 1, 1) = input[7];
  (*this)(2, 2, 2, 2) = input[8];
}

RankTwoTensor
RankFourTensor::innerProductTranspose(const RankTwoTensor & b) const
{
  RankTwoTensor result;

  unsigned int index = 0;
  for (unsigned int ij = 0; ij < N2; ++ij)
  {
    Real bb = b._vals[ij];
    for (unsigned int kl = 0; kl < N2; ++kl)
      result._vals[kl] += _vals[index++] * bb;
  }

  return result;
}

Real
RankFourTensor::sum3x3() const
{
  // summation of Ciijj for i and j ranging from 0 to 2 - used in the volumetric locking correction
  return (*this)(0, 0, 0, 0) + (*this)(0, 0, 1, 1) + (*this)(0, 0, 2, 2) + (*this)(1, 1, 0, 0) +
         (*this)(1, 1, 1, 1) + (*this)(1, 1, 2, 2) + (*this)(2, 2, 0, 0) + (*this)(2, 2, 1, 1) +
         (*this)(2, 2, 2, 2);
}

RealGradient
RankFourTensor::sum3x1() const
{
  // used for volumetric locking correction
  RealGradient a(3);
  a(0) = (*this)(0, 0, 0, 0) + (*this)(0, 0, 1, 1) + (*this)(0, 0, 2, 2); // C0000 + C0011 + C0022
  a(1) = (*this)(1, 1, 0, 0) + (*this)(1, 1, 1, 1) + (*this)(1, 1, 2, 2); // C1100 + C1111 + C1122
  a(2) = (*this)(2, 2, 0, 0) + (*this)(2, 2, 1, 1) + (*this)(2, 2, 2, 2); // C2200 + C2211 + C2222
  return a;
}

bool
RankFourTensor::isSymmetric() const
{
  for (unsigned int i = 1; i < N; ++i)
    for (unsigned int j = 0; j < i; ++j)
      for (unsigned int k = 1; k < N; ++k)
        for (unsigned int l = 0; l < k; ++l)
        {
          // minor symmetries
          if ((*this)(i, j, k, l) != (*this)(j, i, k, l) ||
              (*this)(i, j, k, l) != (*this)(i, j, l, k))
            return false;

          // major symmetry
          if ((*this)(i, j, k, l) != (*this)(k, l, i, j))
            return false;
        }
  return true;
}

bool
RankFourTensor::isIsotropic() const
{
  // prerequisite is symmetry
  if (!isSymmetric())
    return false;

  // inspect shear components
  const Real mu = (*this)(0, 1, 0, 1);
  // ...diagonal
  if ((*this)(1, 2, 1, 2) != mu || (*this)(2, 0, 2, 0) != mu)
    return false;
  // ...off-diagonal
  if ((*this)(2, 0, 1, 2) != 0.0 || (*this)(0, 1, 1, 2) != 0.0 || (*this)(0, 1, 2, 0) != 0.0)
    return false;

  // off diagonal blocks in Voigt
  unsigned int i1 = 0;
  for (unsigned int i = 0; i < N; ++i)
  {
    for (unsigned int j = 0; j < N; ++j)
      if (_vals[i1 + ((j + 1) % N) * N + (j + 2) % N] != 0.0)
        return false;
    i1 += N3 + N2;
  }

  // top left block
  const Real K1 = (*this)(0, 0, 0, 0);
  const Real K2 = (*this)(0, 0, 1, 1);
  if (!MooseUtils::relativeFuzzyEqual(K1 - 4.0 * mu / 3.0, K2 + 2.0 * mu / 3.0))
    return false;
  if ((*this)(1, 1, 1, 1) != K1 || (*this)(2, 2, 2, 2) != K1)
    return false;
  for (unsigned int i = 1; i < N; ++i)
    for (unsigned int j = 0; j < i; ++j)
      if ((*this)(i, i, j, j) != K2)
        return false;

  return true;
}
