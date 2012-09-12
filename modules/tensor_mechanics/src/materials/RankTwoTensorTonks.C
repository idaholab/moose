#include "RankTwoTensorTonks.h"

// Any other includes here
#include <vector>
#include "libmesh.h"
#include "tensor_value.h"
#include "MaterialProperty.h"
#include <ostream>

RankTwoTensorTonks::RankTwoTensorTonks()
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      _vals[i][j] = 0.0;
}

RankTwoTensorTonks::RankTwoTensorTonks(const TypeVector<Real> & row1, const TypeVector<Real> & row2, const TypeVector<Real> & row3) 
{
  // Initialize the TensorTonks matrix from the passed in vectors
  for(unsigned int i=0; i<N; i++)
    _vals[0][i] = row1(i);
  
  for(unsigned int i=0; i<N; i++)
    _vals[1][i] = row2(i);

  for(unsigned int i=0; i<N; i++)
    _vals[2][i] = row3(i);
}

RankTwoTensorTonks::RankTwoTensorTonks(const RankTwoTensorTonks &a)
{
  *this = a;
}

RankTwoTensorTonks::RankTwoTensorTonks(const TypeTensor<Real> &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = a(i,j);
}

Real &
RankTwoTensorTonks::operator()(unsigned int i, unsigned int j)
{
  return _vals[i][j];
}

Real 
RankTwoTensorTonks::operator()(unsigned int i, unsigned int j) const
{
  return _vals[i][j];
}

void
RankTwoTensorTonks::setValue(Real val, unsigned int i, unsigned int j)
{
  _vals[i-1][j-1] = val;
}

Real
RankTwoTensorTonks::getValue(unsigned int i, unsigned int j) const
{
  return _vals[i-1][j-1];
}

TypeVector<Real>
RankTwoTensorTonks::row(const unsigned int r) const
{
  RealVectorValue result;
  for (unsigned int i = 0; i<N; i++)
    result(i) = _vals[r][i];
  
  return result;
}

void
RankTwoTensorTonks::rotate(RealTensorValue &R)
{
  Real temp;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
    {
      temp = 0.0;
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          temp += R(i,k)*R(j,l)*_vals[i][j];
      _vals[i][j] = temp;
    }

}

void
RankTwoTensorTonks::zero()
{
    for(unsigned int i(0); i<N; i++)
      for(unsigned int j(0); j<N; j++)
        _vals[i][j] = 0.0;
}

RankTwoTensorTonks &
RankTwoTensorTonks::operator= (const RankTwoTensorTonks &a)
{
  for(unsigned int i(0); i<N; i++)
      for(unsigned int j(0); j<N; j++)
        _vals[i][j] = a._vals[i][j];
  
  return *this;
}

RankTwoTensorTonks &
RankTwoTensorTonks::operator+=(const RankTwoTensorTonks &a)
{
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = _vals[i][j] + a(i,j);
  return *this;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator+ (const RankTwoTensorTonks &a) const
{
  RankTwoTensorTonks result;
  
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j] + a(i,j);

   return result;
}

RankTwoTensorTonks &
RankTwoTensorTonks::operator-=(const RankTwoTensorTonks &a)
{
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = _vals[i][j] - a(i,j);
  return *this;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator- (const RankTwoTensorTonks &a) const
{
  RankTwoTensorTonks result;
  
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j] - a(i,j);

   return result;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator - () const
{
  RankTwoTensorTonks result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = -_vals[i][j];

  return result;
}  

RankTwoTensorTonks &
RankTwoTensorTonks::operator*=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = _vals[i][j]*a;

  return *this;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator*(const Real &a) const
{
  RankTwoTensorTonks result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j]*a;

  return result;
}

RankTwoTensorTonks &
RankTwoTensorTonks::operator/=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = _vals[i][j]/a;

  return *this;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator/(const Real &a) const
{
  RankTwoTensorTonks result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j]/a;

  return result;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator*(const RankTwoTensorTonks &a) const
{
  RankTwoTensorTonks result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        result(i,k) = _vals[i][j]*a(j,k);

  return result;
}

RankTwoTensorTonks
RankTwoTensorTonks::operator*(const TypeTensor<Real> &a) const
{
  RankTwoTensorTonks result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        result(i,k) = _vals[i][j]*a(j,k);

  return result;
}

Real
RankTwoTensorTonks::doubleContraction(const RankTwoTensorTonks &a)
{
  Real result(0.0);
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result += _vals[i][j]* a(i,j);

  return result;
}

RankTwoTensorTonks
RankTwoTensorTonks::transpose() 
{
  RankTwoTensorTonks result;

  for(unsigned int i=0; i<N; i++)
    for(unsigned int j=0; j<N; j++)
      result(i,j) = _vals[j][i];

  return result;
}

Real
RankTwoTensorTonks::trace()
{
  Real result(0.0);
  
  for(unsigned int i(0); i<N; i++)
      result += _vals[i][i];

  return result;
}

Real
RankTwoTensorTonks::det()
{
  Real result(0.0);
  
  result = _vals[0][0]*(_vals[1][1]*_vals[2][2] - _vals[2][1]*_vals[1][2]);
  result -= _vals[1][0]*(_vals[0][1]*_vals[2][2] - _vals[2][1]*_vals[0][2]);
  result += _vals[2][0]*(_vals[0][1]*_vals[1][2] - _vals[1][1]*_vals[0][2]);

  return result;
}

RankTwoTensorTonks
RankTwoTensorTonks::inverse() 
{
  RankTwoTensorTonks result;

  result(0,0) = _vals[1][1]*_vals[2][2] - _vals[2][1]*_vals[1][2];
  result(0,1) = _vals[0][2]*_vals[2][1] - _vals[0][1]*_vals[2][2];
  result(0,2) = _vals[0][1]*_vals[1][2] - _vals[0][2]*_vals[1][1];
  result(1,0) = _vals[1][2]*_vals[2][0] - _vals[1][0]*_vals[2][2];
  result(1,1) = _vals[0][0]*_vals[2][2] - _vals[0][2]*_vals[2][0];
  result(1,2) = _vals[0][2]*_vals[1][0] - _vals[0][0]*_vals[1][2];
  result(2,0) = _vals[1][0]*_vals[2][1] - _vals[1][1]*_vals[2][0];
  result(2,1) = _vals[0][1]*_vals[2][0] - _vals[0][0]*_vals[2][1];
  result(2,2) = _vals[0][0]*_vals[1][1] - _vals[0][1]*_vals[1][0];

  Real det =  _vals[0][0]*(_vals[1][1]*_vals[2][2] - _vals[2][1]*_vals[1][2]);
  det -= _vals[1][0]*(_vals[0][1]*_vals[2][2] - _vals[2][1]*_vals[0][2]);
  det += _vals[2][0]*(_vals[0][1]*_vals[1][2] - _vals[1][1]*_vals[0][2]);

  result /= det;
  
  return result;
}
