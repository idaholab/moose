#include "SymmElasticityTensor.h"

SymmElasticityTensor::SymmElasticityTensor(const bool constant)
  : _constant(constant),
    _values_computed(false)
{
  for (unsigned i(0); i < 21; ++i)
  {
    _val[i] = 0;
  }
}

void
SymmElasticityTensor::calculate(unsigned int qp)
{
  if(!_constant || !_values_computed)
  {
    calculateEntries(qp);
    _values_computed = true;
  }
}

ColumnMajorMatrix
SymmElasticityTensor::calculateDerivative(unsigned int /*qp*/, unsigned int /*i*/)
{
  ColumnMajorMatrix m(9, 9);
  return m;
}

void
SymmElasticityTensor::multiply( const SymmTensor & x, SymmTensor & b )
{
  const Real xx = x.xx();
  const Real yy = x.yy();
  const Real zz = x.zz();
  const Real xy = x.xy();
  const Real yz = x.yz();
  const Real zx = x.zx();

  b.xx() = _val[ 0]*xx + _val[ 1]*yy + _val[ 2]*zz;
  b.yy() = _val[ 1]*xx + _val[ 6]*yy + _val[ 7]*zz;
  b.zz() = _val[ 2]*xx + _val[ 7]*yy + _val[11]*zz;
  b.xy() = 2*_val[15]*xy;
  b.yz() = 2*_val[18]*yz;
  b.zx() = 2*_val[20]*zx;
}

void
SymmElasticityTensor::generalMultiply( const SymmTensor & x, SymmTensor & b )
{
  const Real xx = x.xx();
  const Real yy = x.yy();
  const Real zz = x.zz();
  const Real xy = x.xy();
  const Real yz = x.yz();
  const Real zx = x.zx();

  b.xx() = _val[ 0]*xx + _val[ 1]*yy + _val[ 2]*zz + 2*(_val[ 3]*xy + _val[ 4]*yz + _val[ 5]*zx);
  b.yy() = _val[ 1]*xx + _val[ 6]*yy + _val[ 7]*zz + 2*(_val[ 8]*xy + _val[ 9]*yz + _val[10]*zx);
  b.zz() = _val[ 2]*xx + _val[ 7]*yy + _val[11]*zz + 2*(_val[12]*xy + _val[13]*yz + _val[14]*zx);
  b.xy() = _val[ 3]*xx + _val[ 8]*yy + _val[12]*zz + 2*(_val[15]*xy + _val[16]*yz + _val[17]*zx);
  b.yz() = _val[ 4]*xx + _val[ 9]*yy + _val[13]*zz + 2*(_val[16]*xy + _val[18]*yz + _val[19]*zx);
  b.zx() = _val[ 5]*xx + _val[10]*yy + _val[14]*zz + 2*(_val[17]*xy + _val[19]*yz + _val[20]*zx);
}

Real
SymmElasticityTensor::stiffness( const unsigned ii, const unsigned jj,
                                 const RealGradient & test,
                                 const RealGradient & phi )
{
  unsigned i(ii);
  unsigned j(jj);
  if ( i > j )
  {
    i = jj;
    j = ii;
  }
  RealGradient b;
  if (0 == i && 0 == j)
  {
    b(0) = _val[ 0]*phi(0) + _val[ 3]*phi(1) + _val[ 5]*phi(2);
    b(1) = _val[ 3]*phi(0) + _val[15]*phi(1) + _val[17]*phi(2);
    b(2) = _val[ 5]*phi(0) + _val[17]*phi(1) + _val[20]*phi(2);
  }
  else if (1 == i && 1 == j)
  {
    b(0) = _val[15]*phi(0) + _val[ 8]*phi(1) + _val[16]*phi(2);
    b(1) = _val[ 8]*phi(0) + _val[ 6]*phi(1) + _val[ 9]*phi(2);
    b(2) = _val[16]*phi(0) + _val[ 9]*phi(2) + _val[18]*phi(2);
  }
  else if (2 == i && 2 == j)
  {
    b(0) = _val[20]*phi(0) + _val[19]*phi(1) + _val[14]*phi(2);
    b(1) = _val[19]*phi(0) + _val[18]*phi(1) + _val[13]*phi(2);
    b(2) = _val[14]*phi(0) + _val[13]*phi(1) + _val[11]*phi(2);
  }
  else if (0 == i && 1 == j)
  {
    b(0) = _val[ 3]*phi(0) + _val[ 1]*phi(1) + _val[ 4]*phi(2);
    b(1) = _val[15]*phi(0) + _val[ 8]*phi(1) + _val[16]*phi(2);
    b(2) = _val[17]*phi(0) + _val[10]*phi(1) + _val[19]*phi(2);
  }
  else if (1 == i && 2 == j)
  {
    b(0) = _val[17]*phi(0) + _val[16]*phi(1) + _val[14]*phi(2);
    b(1) = _val[10]*phi(0) + _val[10]*phi(1) + _val[ 7]*phi(2);
    b(2) = _val[19]*phi(0) + _val[18]*phi(1) + _val[13]*phi(2);
  }
  else if (0 == i && 2 == j)
  {
    b(0) = _val[ 5]*phi(0) + _val[ 4]*phi(1) + _val[ 2]*phi(2);
    b(1) = _val[17]*phi(0) + _val[16]*phi(1) + _val[12]*phi(2);
    b(2) = _val[20]*phi(0) + _val[19]*phi(1) + _val[14]*phi(2);
  }
  else
  {
    mooseError( "Wrong index in stiffness calculation" );
  }
  return test * b;
}

