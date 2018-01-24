/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SymmElasticityTensor.h"
#include <vector>

template <>
void
dataStore(std::ostream & stream, SymmElasticityTensor & set, void * context)
{
  dataStore(stream, set._constant, context);
  dataStore(stream, set._values_computed, context);
  dataStore(stream, set._val, context);
}

template <>
void
dataLoad(std::istream & stream, SymmElasticityTensor & set, void * context)
{
  dataLoad(stream, set._constant, context);
  dataLoad(stream, set._values_computed, context);
  dataLoad(stream, set._val, context);
}

SymmElasticityTensor::SymmElasticityTensor(const bool constant)
  : _constant(constant), _values_computed(false)
{
  for (unsigned i(0); i < 21; ++i)
  {
    _val[i] = 0;
  }
}

void
SymmElasticityTensor::calculate(unsigned int qp)
{
  if (!_constant || !_values_computed)
  {
    calculateEntries(qp);
    _values_computed = true;
  }
}

void
SymmElasticityTensor::calculateEntries(unsigned int /*qp*/)
{
}

void
SymmElasticityTensor::multiply(const SymmTensor & x, SymmTensor & b) const
{
  const Real xx = x.xx();
  const Real yy = x.yy();
  const Real zz = x.zz();
  const Real xy = x.xy();
  const Real yz = x.yz();
  const Real zx = x.zx();

  b.xx() =
      _val[0] * xx + _val[1] * yy + _val[2] * zz + 2 * (_val[3] * xy + _val[4] * yz + _val[5] * zx);
  b.yy() = _val[1] * xx + _val[6] * yy + _val[7] * zz +
           2 * (_val[8] * xy + _val[9] * yz + _val[10] * zx);
  b.zz() = _val[2] * xx + _val[7] * yy + _val[11] * zz +
           2 * (_val[12] * xy + _val[13] * yz + _val[14] * zx);
  b.xy() = _val[3] * xx + _val[8] * yy + _val[12] * zz +
           2 * (_val[15] * xy + _val[16] * yz + _val[17] * zx);
  b.yz() = _val[4] * xx + _val[9] * yy + _val[13] * zz +
           2 * (_val[16] * xy + _val[18] * yz + _val[19] * zx);
  b.zx() = _val[5] * xx + _val[10] * yy + _val[14] * zz +
           2 * (_val[17] * xy + _val[19] * yz + _val[20] * zx);
}

SymmTensor SymmElasticityTensor::operator*(const SymmTensor & x) const
{
  SymmTensor b;
  multiply(x, b);
  return b;
}

Real
SymmElasticityTensor::stiffness(const unsigned int i,
                                const unsigned int j,
                                const RealGradient & test,
                                const RealGradient & phi) const
{
  RealGradient b;
  if (0 == i && 0 == j)
  {
    b(0) = _val[0] * phi(0) + _val[3] * phi(1) + _val[5] * phi(2);
    b(1) = _val[3] * phi(0) + _val[15] * phi(1) + _val[17] * phi(2);
    b(2) = _val[5] * phi(0) + _val[17] * phi(1) + _val[20] * phi(2);
  }
  else if (1 == i && 1 == j)
  {
    b(0) = _val[15] * phi(0) + _val[8] * phi(1) + _val[16] * phi(2);
    b(1) = _val[8] * phi(0) + _val[6] * phi(1) + _val[9] * phi(2);
    b(2) = _val[16] * phi(0) + _val[9] * phi(1) + _val[18] * phi(2);
  }
  else if (2 == i && 2 == j)
  {
    b(0) = _val[20] * phi(0) + _val[19] * phi(1) + _val[14] * phi(2);
    b(1) = _val[19] * phi(0) + _val[18] * phi(1) + _val[13] * phi(2);
    b(2) = _val[14] * phi(0) + _val[13] * phi(1) + _val[11] * phi(2);
  }
  else if (0 == i && 1 == j)
  {
    b(0) = _val[3] * phi(0) + _val[1] * phi(1) + _val[4] * phi(2);
    b(1) = _val[15] * phi(0) + _val[8] * phi(1) + _val[16] * phi(2);
    b(2) = _val[17] * phi(0) + _val[10] * phi(1) + _val[19] * phi(2);
  }
  else if (1 == i && 0 == j)
  {
    b(0) = _val[3] * phi(0) + _val[15] * phi(1) + _val[17] * phi(2);
    b(1) = _val[1] * phi(0) + _val[8] * phi(1) + _val[10] * phi(2);
    b(2) = _val[4] * phi(0) + _val[16] * phi(1) + _val[19] * phi(2);
  }
  else if (1 == i && 2 == j)
  {
    b(0) = _val[17] * phi(0) + _val[16] * phi(1) + _val[12] * phi(2);
    b(1) = _val[10] * phi(0) + _val[9] * phi(1) + _val[7] * phi(2);
    b(2) = _val[19] * phi(0) + _val[18] * phi(1) + _val[13] * phi(2);
  }
  else if (2 == i && 1 == j)
  {
    b(0) = _val[17] * phi(0) + _val[10] * phi(1) + _val[19] * phi(2);
    b(1) = _val[16] * phi(0) + _val[9] * phi(1) + _val[18] * phi(2);
    b(2) = _val[12] * phi(0) + _val[7] * phi(1) + _val[13] * phi(2);
  }
  else if (0 == i && 2 == j)
  {
    b(0) = _val[5] * phi(0) + _val[4] * phi(1) + _val[2] * phi(2);
    b(1) = _val[17] * phi(0) + _val[16] * phi(1) + _val[12] * phi(2);
    b(2) = _val[20] * phi(0) + _val[19] * phi(1) + _val[14] * phi(2);
  }
  else if (2 == i && 0 == j)
  {
    b(0) = _val[5] * phi(0) + _val[17] * phi(1) + _val[20] * phi(2);
    b(1) = _val[4] * phi(0) + _val[16] * phi(1) + _val[19] * phi(2);
    b(2) = _val[2] * phi(0) + _val[12] * phi(1) + _val[14] * phi(2);
  }
  else
  {
    std::stringstream s;
    s << "Wrong index in stiffness calculation: ";
    s << i << " " << j;
    mooseError(s.str());
  }
  return test * b;
}

void
SymmElasticityTensor::convertFrom9x9(const ColumnMajorMatrix & input)
{
  if (input.numEntries() != 81)
  {
    mooseError("Cannot convert from ColumnMajorMatrix (wrong size)");
  }

  _val[0] = input(0, 0);
  _val[1] = input(0, 4);
  _val[2] = input(0, 8);
  _val[3] = input(0, 1);
  _val[4] = input(0, 5);
  _val[5] = input(0, 2);

  _val[6] = input(4, 4);
  _val[7] = input(4, 8);
  _val[8] = input(4, 3);
  _val[9] = input(4, 5);
  _val[10] = input(4, 6);

  _val[11] = input(8, 8);
  _val[12] = input(8, 3);
  _val[13] = input(8, 5);
  _val[14] = input(8, 6);

  _val[15] = input(1, 1);
  _val[16] = input(1, 5);
  _val[17] = input(1, 2);

  _val[18] = input(5, 5);
  _val[19] = input(5, 6);

  _val[20] = input(2, 2);
}

void
SymmElasticityTensor::convertFrom6x6(const ColumnMajorMatrix & input)
{
  if (input.numEntries() != 36)
  {
    mooseError("Cannot convert from ColumnMajorMatrix (wrong size)");
  }

  _val[0] = input(0, 0);
  _val[1] = input(0, 1);
  _val[2] = input(0, 2);
  _val[3] = input(0, 3);
  _val[4] = input(0, 4);
  _val[5] = input(0, 5);

  _val[6] = input(1, 1);
  _val[7] = input(1, 2);
  _val[8] = input(1, 3);
  _val[9] = input(1, 4);
  _val[10] = input(1, 5);

  _val[11] = input(2, 2);
  _val[12] = input(2, 3);
  _val[13] = input(2, 4);
  _val[14] = input(2, 5);

  _val[15] = input(3, 3);
  _val[16] = input(3, 4);
  _val[17] = input(3, 5);

  _val[18] = input(4, 4);
  _val[19] = input(4, 5);

  _val[20] = input(5, 5);
}

ColumnMajorMatrix
SymmElasticityTensor::columnMajorMatrix6x6() const
{
  ColumnMajorMatrix cmm(6, 6);
  unsigned count(0);
  for (unsigned i(0); i < 6; ++i)
  {
    for (unsigned j(i); j < 6; ++j)
    {
      cmm(i, j) = cmm(j, i) = _val[count++];
    }
  }
  return cmm;
}

ColumnMajorMatrix
SymmElasticityTensor::columnMajorMatrix9x9() const
{
  ColumnMajorMatrix cmm(9, 9);
  cmm(0, 0) = _val[0];
  cmm(0, 1) = cmm(1, 0) = _val[3];
  cmm(0, 2) = cmm(2, 0) = _val[5];
  cmm(0, 3) = cmm(3, 0) = _val[3];
  cmm(0, 4) = cmm(4, 0) = _val[1];
  cmm(0, 5) = cmm(5, 0) = _val[4];
  cmm(0, 6) = cmm(6, 0) = _val[5];
  cmm(0, 7) = cmm(7, 0) = _val[4];
  cmm(0, 8) = cmm(8, 0) = _val[2];

  cmm(1, 1) = _val[15];
  cmm(1, 2) = cmm(2, 1) = _val[17];
  cmm(1, 3) = cmm(3, 1) = _val[15];
  cmm(1, 4) = cmm(4, 1) = _val[8];
  cmm(1, 5) = cmm(5, 1) = _val[16];
  cmm(1, 6) = cmm(6, 1) = _val[17];
  cmm(1, 7) = cmm(7, 1) = _val[16];
  cmm(1, 8) = cmm(8, 1) = _val[12];

  cmm(2, 2) = _val[20];
  cmm(2, 3) = cmm(3, 2) = _val[17];
  cmm(2, 4) = cmm(4, 2) = _val[10];
  cmm(2, 5) = cmm(5, 2) = _val[19];
  cmm(2, 6) = cmm(6, 2) = _val[20];
  cmm(2, 7) = cmm(7, 2) = _val[19];
  cmm(2, 8) = cmm(8, 2) = _val[14];

  cmm(3, 3) = _val[15];
  cmm(3, 4) = cmm(4, 3) = _val[8];
  cmm(3, 5) = cmm(5, 3) = _val[16];
  cmm(3, 6) = cmm(6, 3) = _val[17];
  cmm(3, 7) = cmm(7, 3) = _val[16];
  cmm(3, 8) = cmm(8, 3) = _val[12];

  cmm(4, 4) = _val[6];
  cmm(4, 5) = cmm(5, 4) = _val[9];
  cmm(4, 6) = cmm(6, 4) = _val[10];
  cmm(4, 7) = cmm(7, 4) = _val[9];
  cmm(4, 8) = cmm(8, 4) = _val[7];

  cmm(5, 5) = _val[18];
  cmm(5, 6) = cmm(6, 5) = _val[19];
  cmm(5, 7) = cmm(7, 5) = _val[18];
  cmm(5, 8) = cmm(8, 5) = _val[13];

  cmm(6, 6) = _val[20];
  cmm(6, 7) = cmm(7, 6) = _val[19];
  cmm(6, 8) = cmm(8, 6) = _val[14];

  cmm(7, 7) = _val[18];
  cmm(7, 8) = cmm(8, 7) = _val[13];

  cmm(8, 8) = _val[11];

  return cmm;
}

std::ostream &
operator<<(std::ostream & stream, const SymmElasticityTensor & obj)
{
  stream << "SymmElasticityTensor:\n"
         << std::setprecision(6) << std::setw(13) << obj._val[0] << "\t" << std::setw(13)
         << obj._val[1] << "\t" << std::setw(13) << obj._val[2] << "\t" << std::setw(13)
         << obj._val[3] << "\t" << std::setw(13) << obj._val[4] << "\t" << std::setw(13)
         << obj._val[5] << "\n"
         << "\t\t" << std::setw(13) << obj._val[6] << "\t" << std::setw(13) << obj._val[7] << "\t"
         << std::setw(13) << obj._val[8] << "\t" << std::setw(13) << obj._val[9] << "\t"
         << std::setw(13) << obj._val[10] << "\n"
         << "\t\t\t\t" << std::setw(13) << obj._val[11] << "\t" << std::setw(13) << obj._val[12]
         << "\t" << std::setw(13) << obj._val[13] << "\t" << std::setw(13) << obj._val[14] << "\n"
         << "\t\t\t\t\t\t" << std::setw(13) << obj._val[15] << "\t" << std::setw(13) << obj._val[16]
         << "\t" << std::setw(13) << obj._val[17] << "\t"
         << "\n"
         << "\t\t\t\t\t\t\t\t" << std::setw(13) << obj._val[18] << "\t" << std::setw(13)
         << obj._val[19] << "\n"
         << "\t\t\t\t\t\t\t\t\t\t" << std::setw(13) << obj._val[20] << std::endl;
  return stream;
}

SymmElasticityTensor
SymmElasticityTensor::calculateDerivative(unsigned int /*qp*/, unsigned int /*i*/)
{
  return SymmElasticityTensor();
}

SymmElasticityTensor SymmElasticityTensor::operator*(Real x) const
{
  SymmElasticityTensor fred(*this);
  fred *= x;
  return fred;
}

void
SymmElasticityTensor::form9x9Rotation(const ColumnMajorMatrix & R_3x3,
                                      ColumnMajorMatrix & R_9x9) const
{
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      for (int k = 0; k < 3; ++k)
      {
        for (int l = 0; l < 3; ++l)
        {
          R_9x9(((i * 3) + k), ((j * 3) + l)) = R_3x3(i, j) * R_3x3(k, l);
        }
      }
    }
  }
}

void
SymmElasticityTensor::rotateFromLocalToGlobal(const ColumnMajorMatrix & R)
{
  convertFrom9x9((R * columnMajorMatrix9x9()) * R.transpose());
}

void
SymmElasticityTensor::rotateFromGlobalToLocal(const ColumnMajorMatrix & R)
{
  convertFrom9x9(R.transpose() * (columnMajorMatrix9x9() * R));
}

void
SymmElasticityTensor::adjustForCracking(const RealVectorValue & /*crack_flags*/)
{
  mooseError("adjustForCracking method not defined");
}

void
SymmElasticityTensor::adjustForCrackingWithShearRetention(const RealVectorValue & /*crack_flags*/)
{
  mooseError("adjustForCrackingWithShearRetention method not defined");
}

void
SymmElasticityTensor::fillFromInputVector(std::vector<Real> input, bool all)
{
  if ((all == true && input.size() != 21) || (all == false && input.size() != 9))
    mooseError("Please check the number of entries in the stiffness input vector.");

  if (all == true)
  {
    for (int i = 0; i < 21; i++)
      _val[i] = input[i];
  }
  else
  {
    _val[0] = input[0];  // C1111
    _val[1] = input[1];  // C1122
    _val[2] = input[2];  // C1133
    _val[6] = input[3];  // C2222
    _val[7] = input[4];  // C2233
    _val[11] = input[5]; // C3333
    _val[15] = input[6]; // C2323
    _val[18] = input[7]; // C1313
    _val[20] = input[8]; // C1212
  }
}

Real
SymmElasticityTensor::valueAtIndex(int i) const
{
  return _val[i];
}

Real
SymmElasticityTensor::sum_3x3() const
{
  // summation of Cij for i and j ranging from 0 to 2 - used in the volumetric locking correction
  return _val[0] + 2 * (_val[1] + _val[2] + _val[7]) + _val[6] + _val[11];
}

RealGradient
SymmElasticityTensor::sum_3x1() const
{
  // used for volumetric locking correction
  RealGradient a(3);
  a(0) = _val[0] + _val[1] + _val[2];  // C00 + C01 + C02
  a(1) = _val[1] + _val[6] + _val[7];  // C10 + C11 + C12
  a(2) = _val[2] + _val[7] + _val[11]; // C20 + C21 + C22
  return a;
}
