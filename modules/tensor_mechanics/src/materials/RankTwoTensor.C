#include "RankTwoTensor.h"

// Any other includes here
#include <vector>
#include "libmesh.h"
#include "MaterialProperty.h"

RankTwoTensor::RankTwoTensor() :
    _vals(),
    _euler_angle(3)
{
  _vals.resize(3);
  for (unsigned int i(0); i<3; i++)
  {
    _vals[i].resize(3);
    for (unsigned int j(0); j<3; j++)
    {
      //  _vals[i].push_back(0.0);
      _vals[i][j] = 0.0;
    }
  }
  
  for(unsigned int i(0); i<3, i++)
    _euler_angle[i] = 0.0;
}

RankTwoTensor::RankTwoTensor(const RankTwoTensor &a)
{
  *this = a;
}

void
RankTwoTensor::fillFromInputVector(const std::vector<Real> input)
{   
  if (input.size() != 6)
    mooseError("Please check the number of entries in the eigenstrain input vector");

  _vals[0][0] = input[0]; //S11
  _vals[1][1] = input[1]; //S22
  _vals[2][2] = input[2]; //S33
  _vals[1][2] = _vals[2][1] = input[3]; //S23
  _vals[0][2] = _vals[2][0] = input[4]; //S13
  _vals[0][1] = _vals[1][0] = input[5]; //S12
} 

void
RankTwoTensor::setValue(Real val, int i, int j)
{
  _vals[i-1][j-1] = val;
}

Real
RankTwoTensor::getValue(int i, int j) const
{
  return _vals[i-1][j-1];
}

Real
RankTwoTensor::rowDot(const unsigned int r, const libMesh::TypeVector<Real> & v) const
{
  mooseAssert(LIBMESH_DIM == 3, "Incompatible sizes");
  if (0 == r)
  {  
    return _vals[0][0]*v(0) + _vals[0][1]*v(1) + _vals[0][2]*v(2);
    //return _xx * v(0) + _xy * v(1) + _zx * v(2);
  }
  else if (1 == r)
  {
    return _vals[1][0]*v(0) + _vals[1][1]*v(1) + _vals[1][2]*v(2);
    //return _xy * v(0) + _yy * v(1) + _yz * v(2);
  }
  else if (2 == r)
  {
    return _vals[2][0]*v(0) + _vals[2][1]*v(1) + _vals[2][2]*v(2);
    //return _zx * v(0) + _yz * v(1) + _zz * v(2);
  }
  else
  {
    mooseError( "Incorrect row" );
  }
  return 0.0;
}


void
RankTwoTensor::selfRotate(const Real a1, const Real a2, const Real a3)
{
}

RankTwoTensor
RankTwoTensor::rotate(const Real a1, const Real a2, const Real a3)
{
}


void
RankTwoTensor::setFirstEulerAngle(const Real a1)
{
  _euler_angle[0] = a1;
}

void
RankTwoTensor::setSecondEulerAngle(const Real a2)
{
  _euler_angle[1] = a2;
}

void
RankTwoTensor::setThirdEulerAngle(const Real a3)
{
  _euler_angle[2] = a3;
}

Real
RankTwoTensor::firstEulerAngle()
{
  return _euler_angle[0];
}

Real
RankTwoTensor::secondEulerAngle()
{
  return _euler_angle[1];
}

Real
RankTwoTensor::thirdEulerAngle()
{
  return _euler_angle[2];
}
