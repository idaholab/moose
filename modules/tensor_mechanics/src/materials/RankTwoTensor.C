#include "RankTwoTensor.h"

// Any other includes here
#include <vector>
#include "libmesh/libmesh.h"
#include "libmesh/tensor_value.h"
#include "MaterialProperty.h"
#include <ostream>

RankTwoTensor::RankTwoTensor()
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      _vals[i][j] = 0.0;
}

RankTwoTensor::RankTwoTensor(const TypeVector<Real> & row1, const TypeVector<Real> & row2, const TypeVector<Real> & row3)
{
  // Initialize the Tensor matrix from the passed in vectors
  for(unsigned int i=0; i<N; i++)
    _vals[0][i] = row1(i);

  for(unsigned int i=0; i<N; i++)
    _vals[1][i] = row2(i);

  for(unsigned int i=0; i<N; i++)
    _vals[2][i] = row3(i);
}

RankTwoTensor::RankTwoTensor(const RankTwoTensor &a)
{
  *this = a;
}

RankTwoTensor::RankTwoTensor(const TypeTensor<Real> &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] = a(i,j);
}

Real &
RankTwoTensor::operator()(unsigned int i, unsigned int j)
{
  return _vals[i][j];
}

Real
RankTwoTensor::operator()(unsigned int i, unsigned int j) const
{
  return _vals[i][j];
}

void
RankTwoTensor::fillFromInputVector(const std::vector<Real> input)
{
  if (input.size() == 6)
  {
    Moose::out << "Rank 2 tensor input size =" << input.size() << std::endl;

    _vals[0][0] = input[0]; //S11
    _vals[1][1] = input[1]; //S22
    _vals[2][2] = input[2]; //S33
    _vals[1][2] = _vals[2][1] = input[3]; //S23
    _vals[0][2] = _vals[2][0] = input[4]; //S13
    _vals[0][1] = _vals[1][0] = input[5]; //S12
  }
  else if (input.size() == 9)
  {
    Moose::out << "Rank 2 tensor input size =" << input.size() << std::endl;

    _vals[0][0] = input[0]; //S11
    _vals[1][0] = input[1]; //S21
    _vals[2][0] = input[2]; //S31
    _vals[0][1] = input[3]; //S12
    _vals[1][1] = input[4]; //S22
    _vals[2][1] = input[5]; //S32
    _vals[0][2] = input[6]; //S13
    _vals[1][2] = input[7]; //S23
    _vals[2][2] = input[8]; //S33
  }
  else
    mooseError("Please check the number of entries in the eigenstrain input vector");
}

void
RankTwoTensor::setValue(Real val, unsigned int i, unsigned int j)
{
  _vals[i-1][j-1] = val;
}

Real
RankTwoTensor::getValue(unsigned int i, unsigned int j) const
{
  return _vals[i-1][j-1];
}

TypeVector<Real>
RankTwoTensor::row(const unsigned int r) const
{
  RealVectorValue result;
  for (unsigned int i = 0; i<N; i++)
    result(i) = _vals[r][i];

  return result;
}

void
RankTwoTensor::rotate(RealTensorValue &R)
{
  Real temp;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
    {
      temp = 0.0;
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          temp += R(i,k)*R(j,l)*_vals[k][l];
      _vals[i][j] = temp;
    }

}

void
RankTwoTensor::rotate(RankTwoTensor &R)
{
  Real temp;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
    {
      temp = 0.0;
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          temp += R(i,k)*R(j,l)*_vals[k][l];
      _vals[i][j] = temp;
 }
}

RankTwoTensor
RankTwoTensor::rotateXyPlane(const Real a)
{
  RankTwoTensor b;

  Real x, y, xy;
  x = _vals[0][0]*std::cos(a)*std::cos(a)
    + _vals[1][1]*std::sin(a)*std::sin(a)
    + 2.0*_vals[0][1]*std::cos(a)*std::sin(a);

  y = _vals[0][0]*std::sin(a)*std::sin(a)
    + _vals[1][1]*std::cos(a)*std::cos(a)
    - 2.0*_vals[0][1]*std::cos(a)*std::sin(a);

  xy = 2.0*(_vals[1][1] - _vals[0][0])*std::cos(a)*std::sin(a)
     + 2.0*_vals[0][1]*(std::cos(a)*std::cos(a) - std::sin(a)*std::sin(a));

  xy /= 2.0;


  /* Moose::out<<"x="<<x<<std::endl;
  Moose::out<<"y="<<y<<std::endl;
  Moose::out<<"xy="<<xy<<std::endl; */

  b.setValue(x, 1, 1);
  b.setValue(y, 2, 2);
  b.setValue(xy, 1, 2);
  b.setValue(xy, 2, 1);
  b.setValue(_vals[0][2], 1, 3);
  b.setValue(_vals[2][0], 3, 1);
  b.setValue(_vals[1][2], 2, 3);
  b.setValue(_vals[2][1], 3, 2);
  b.setValue(_vals[2][2], 3, 3);

  return b;
}

void
RankTwoTensor::zero()
{
    for(unsigned int i(0); i<N; i++)
      for(unsigned int j(0); j<N; j++)
        _vals[i][j] = 0.0;
}

RankTwoTensor &
RankTwoTensor::operator= (const RankTwoTensor &a)
{
  for(unsigned int i(0); i<N; i++)
      for(unsigned int j(0); j<N; j++)
        _vals[i][j] = a._vals[i][j];

  return *this;
}

RankTwoTensor &
RankTwoTensor::operator+=(const RankTwoTensor &a)
{
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] += a(i,j);
  return *this;
}

RankTwoTensor
RankTwoTensor::operator+ (const RankTwoTensor &a) const
{
  RankTwoTensor result;

   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j] + a(i,j);

   return result;
}

RankTwoTensor &
RankTwoTensor::operator-=(const RankTwoTensor &a)
{
   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] -= a(i,j);
  return *this;
}

RankTwoTensor
RankTwoTensor::operator- (const RankTwoTensor &a) const
{
  RankTwoTensor result;

   for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j] - a(i,j);

   return result;
}

RankTwoTensor
RankTwoTensor::operator - () const
{
  RankTwoTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = -_vals[i][j];

  return result;
}

RankTwoTensor &
RankTwoTensor::operator*=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] *= a;

  return *this;
}

RankTwoTensor
RankTwoTensor::operator*(const Real &a) const
{
  RankTwoTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j]*a;

  return result;
}

RankTwoTensor &
RankTwoTensor::operator/=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      _vals[i][j] /= a;

  return *this;
}

RankTwoTensor
RankTwoTensor::operator/(const Real &a) const
{
  RankTwoTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result(i,j) = _vals[i][j]/a;

  return result;
}

RankTwoTensor &
RankTwoTensor::operator*=(const RankTwoTensor &a)
{
  RankTwoTensor & s = (*this);

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        _vals[i][j] += s(i,j)*a(j,k);

  return *this;
}

RankTwoTensor
RankTwoTensor::operator*(const RankTwoTensor &a) const
{
  RankTwoTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        result(i,k) += _vals[i][j]*a(j,k);

  return result;
}

RankTwoTensor
RankTwoTensor::operator*(const TypeTensor<Real> &a) const
{
  RankTwoTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        result(i,k) += _vals[i][j]*a(j,k);

  return result;
}

Real
RankTwoTensor::doubleContraction(const RankTwoTensor &a)
{
  Real result(0.0);

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      result += _vals[i][j]* a(i,j);

  return result;
}

RankTwoTensor
RankTwoTensor::transpose()
{
  RankTwoTensor result;

  for(unsigned int i=0; i<N; i++)
    for(unsigned int j=0; j<N; j++)
      result(i,j) = _vals[j][i];

  return result;
}

Real
RankTwoTensor::trace()
{
  Real result(0.0);

  for(unsigned int i(0); i<N; i++)
      result += _vals[i][i];

  return result;
}

Real
RankTwoTensor::det()
{
  Real result(0.0);

  result = _vals[0][0]*(_vals[1][1]*_vals[2][2] - _vals[2][1]*_vals[1][2]);
  result -= _vals[1][0]*(_vals[0][1]*_vals[2][2] - _vals[2][1]*_vals[0][2]);
  result += _vals[2][0]*(_vals[0][1]*_vals[1][2] - _vals[1][1]*_vals[0][2]);

  return result;
}

RankTwoTensor
RankTwoTensor::inverse()
{
  RankTwoTensor result;

  result(0,0) = _vals[1][1]*_vals[2][2] - _vals[2][1]*_vals[1][2];
  result(0,1) = _vals[0][2]*_vals[2][1] - _vals[0][1]*_vals[2][2];
  result(0,2) = _vals[0][1]*_vals[1][2] - _vals[0][2]*_vals[1][1];
  result(1,0) = _vals[1][2]*_vals[2][0] - _vals[1][0]*_vals[2][2];
  result(1,1) = _vals[0][0]*_vals[2][2] - _vals[0][2]*_vals[2][0];
  result(1,2) = _vals[0][2]*_vals[1][0] - _vals[0][0]*_vals[1][2];
  result(2,0) = _vals[1][0]*_vals[2][1] - _vals[1][1]*_vals[2][0];
  result(2,1) = _vals[0][1]*_vals[2][0] - _vals[0][0]*_vals[2][1];
  result(2,2) = _vals[0][0]*_vals[1][1] - _vals[0][1]*_vals[1][0];

  Real det = (*this).det();

  if (det == 0)
    mooseError("Rank Two Tensor is singular");

  result /= det;

  return result;
}

void
RankTwoTensor::print()
{
  for(unsigned int i=0; i<N; i++)
  {
    for(unsigned int j=0; j<N; j++)
      Moose::out << std::setw(15) <<_vals[i][j]<<" ";
    Moose::out <<std::endl;
  }
}

void
RankTwoTensor::addIa(const Real &a)
{
  for(unsigned int i=0; i<N; i++)
    _vals[i][i] += a;
}

Real
RankTwoTensor::L2norm()
{

  Real norm;
  norm=0.0;


  for(unsigned int i=0; i<N; i++)
    for(unsigned int j=0; j<N; j++)
      norm+=_vals[i][j]*_vals[i][j];

  norm=pow(norm,0.5);

  return norm;

}
void
RankTwoTensor::surfaceFillFromInputVector(const std::vector<Real> input)
  {
    if(input.size() == 4)
{
    // initialize with zeros
    this->zero();
    _vals[0][0] = input[0];
    _vals[0][1] = input[1];
    _vals[1][0] = input[2];
    _vals[1][1] = input[3];
 /*   // pad the rest of the entries with zeros
    _vals[2][0]=_vals[0][2]=0.;
    _vals[2][1]=_vals[1][2]=0.;
    _vals[2][2]=0.;
*/
  }
  else
   mooseError("please provide correct number of values for surface RankTwoTensor initialization.");
  }
