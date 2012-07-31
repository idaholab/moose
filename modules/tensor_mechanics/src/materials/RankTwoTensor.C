#include "RankTwoTensor.h"

// Any other includes here
#include <vector>
#include "libmesh.h"
#include "MaterialProperty.h"
#include <ostream>

RankTwoTensor::RankTwoTensor() :
    _vals(),
    _euler_angle(3),
    _rotation_matrix()
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
  
  for(unsigned int i(0); i<3; i++)
    _euler_angle[i] = 0.0;

  _rotation_matrix.resize(3);
  for(unsigned int i(0); i<3; i++)
  {
    _rotation_matrix[i].resize(3);
    for(unsigned int j(0); j<3; j++)
    {
      _rotation_matrix[i][j] = 0.0;
    }
  }
  
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
  setFirstEulerAngle(a1);
  setSecondEulerAngle(a2);
  setThirdEulerAngle(a3);
  
  setRotationMatrix();
  
  Real temp;

  //slide down the endless for loop rainbow!
  for(int i(0); i<3; i++)
  {
    for(int j(0); j<3; j++)
    {
      temp = 0.0;
      for(int k(0); k<3; k++)
      {
        for(int l(0); l<3; l++)
        {
          temp += _rotation_matrix[i][k]*_rotation_matrix[j][l]*_vals[i][j];
        }
      }
      _vals[i][j] = temp;
    }
  }

}

RankTwoTensor
RankTwoTensor::rotate(const Real a1, const Real a2, const Real a3)
{
  RankTwoTensor a;

  setFirstEulerAngle(a1);
  setSecondEulerAngle(a2);
  setThirdEulerAngle(a3);

  setRotationMatrix();

  Real temp;
  
  for(int i(0); i<3; i++)
  {
    for(int j(0); j<3; j++)
    {
      temp = 0.0;
      for(int k(0); k<3; k++)
      {
        for(int l(0); l<3; l++)
        {
          temp += _rotation_matrix[i][k]*_rotation_matrix[j][l]*_vals[i][j];
        }
      }
      a.setValue(temp, i+1, j+1);
    }
  }

  return a;
}

void
RankTwoTensor::setRotationMatrix()
{
  Real phi_1 = _euler_angle[0]*(libMesh::pi/180.0);
  Real phi_2 = _euler_angle[1]*(libMesh::pi/180.0);
  Real phi_3 = _euler_angle[2]*(libMesh::pi/180.0);

  Real c1 = std::cos(phi_1);
  Real c2 = std::cos(phi_2);
  Real c3 = std::cos(phi_3);

  Real s1 = std::sin(phi_1);
  Real s2 = std::sin(phi_2);
  Real s3 = std::sin(phi_3);

//doing a Z1, X2, Z3 rotation
  
  _rotation_matrix[0][0] = c1*c3 - c2*s1*s3;
  _rotation_matrix[0][1] = -c1*s3 - c2*c3*s1;
  _rotation_matrix[0][2] = s1*s2;

  _rotation_matrix[1][0] = c3*s1 + c1*c2*s3;
  _rotation_matrix[1][1] = c1*c2*c3 - s1*s3;
  _rotation_matrix[1][2] = -c1*s2;
  
  _rotation_matrix[2][0] = s2*s3;
  _rotation_matrix[2][1] = c3*s2;
  _rotation_matrix[2][2] = c2;
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
RankTwoTensor::firstEulerAngle() const
{
  return _euler_angle[0];
}

Real
RankTwoTensor::secondEulerAngle() const
{
  return _euler_angle[1];
}

Real
RankTwoTensor::thirdEulerAngle() const
{
  return _euler_angle[2];
}

void
RankTwoTensor::zero()
{
    for(unsigned int i(0); i<3; i++)
      for(unsigned int j(0); j<3; j++)
        _vals[i][j] = 0.0;
}

RankTwoTensor &
RankTwoTensor::operator= (const RankTwoTensor &a)
{
 _vals = a._vals;
 _euler_angle = a._euler_angle;
 _rotation_matrix = a._rotation_matrix;
 return *this;
}

RankTwoTensor &
RankTwoTensor::operator+=(const RankTwoTensor &a)
{
   for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      _vals[i][j] = _vals[i][j] + a.getValue(i+1, j+1);
  return *this;
}

RankTwoTensor
RankTwoTensor::operator- (const RankTwoTensor &a) const
{
  RankTwoTensor result;
  
   for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      result.setValue(_vals[i][j]+a.getValue(i+1,j+1), i+1, j+1);

   return result;
}

RankTwoTensor
RankTwoTensor::operator*(const Real &a)
{
  RankTwoTensor result;

  for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      result.setValue(_vals[i][j]*a, i+1, j+1);

  return result;
}

Real
RankTwoTensor::doubleContraction(const RankTwoTensor &a)
{
  Real result(0.0);
  
  for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      result += _vals[i][j]* a.getValue(i+1, j+1);

  return result;
}
