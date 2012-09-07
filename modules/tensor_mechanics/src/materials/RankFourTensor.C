#include "RankFourTensor.h"

// Any other includes here
#include <vector>
#include "MaterialProperty.h"
#include "libmesh.h"
#include <ostream>


RankFourTensor::RankFourTensor() :
    _vals(),
    _euler_angle(),
    _rotation_matrix()
{
  _vals.resize(3);
  for (unsigned int i(0); i<3; i++)
  {
    _vals[i].resize(3);
    for(unsigned int j(0); j<3; j++)
    {
      _vals[i][j].resize(3);
      for(unsigned int k(0); k<3; k++)
      {
        _vals[i][j][k].resize(3);
        for(unsigned int l(0); l<3; l++)
        {
          _vals[i][j][k][l] = 0.0;
       }
      }
    }
  }

  _euler_angle.resize(3);
  for(unsigned int i(0); i< 3; i++)
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

RankFourTensor::RankFourTensor(const RankFourTensor &a)
{
  *this = a;
}

void
RankFourTensor::fillFromInputVector(const std::vector<Real> input, bool all)
{
  if((all == true && input.size() != 21) || (all == false && input.size() != 9 ))
    mooseError("Please check the number of entries in the stiffness input vector.");
  
  zero();
  
  if(all == true)
  {
    _vals[0][0][0][0] = input[0]; //C1111
    _vals[0][0][1][1] = input[1]; //C1122
    _vals[0][0][2][2] = input[2]; //C1133
    _vals[0][0][1][2] = input[3]; //C1123
    _vals[0][0][0][2] = input[4]; //C1113
    _vals[0][0][0][1] = input[5]; //C1112

    _vals[1][1][1][1] = input[6]; //C2222
    _vals[1][1][2][2] = input[7]; //C2233
    _vals[1][1][1][2] = input[8]; //C2223
    _vals[0][2][1][1] = input[9]; //C2213  //flipped for filling purposes
    _vals[0][1][1][1] = input[10]; //C2212 //flipped for filling purposes

    _vals[2][2][2][2] = input[11]; //C3333
    _vals[1][2][2][2] = input[12]; //C3323 //flipped for filling purposes
    _vals[0][2][2][2] = input[13]; //C3313 //flipped for filling purposes
    _vals[0][1][2][2] = input[14]; //C3312 //flipped for filling purposes
    
    _vals[1][2][1][2] = input[15]; //C2323
    _vals[0][2][1][2] = input[16]; //C2313 //flipped for filling purposes
    _vals[0][1][1][2] = input[17]; //C2312 //flipped for filling purposes
     
    _vals[0][2][0][2] = input[18]; //C1313
    _vals[0][1][0][2] = input[19]; //C1312 //flipped for filling purposes
    
    _vals[0][1][0][1] = input[20]; //C1212
  }
  else
  {
    _vals[0][0][0][0] = input[0];   //C1111
    _vals[0][0][1][1] = input[1];   //C1122
    _vals[0][0][2][2] = input[2];   //C1133
    _vals[1][1][1][1] = input[3];   //C2222
    _vals[1][1][2][2] = input[4];   //C2233
    _vals[2][2][2][2] = input[5];  //C3333
    _vals[1][2][1][2] = input[6];  //C2323
    _vals[0][2][0][2] = input[7];  //C1313
    _vals[0][1][0][1] = input[8];  //C1212
  }
  
  //fill in from symmetry relations
  for(unsigned int i(0); i<3; i++)
  {
    for(unsigned int j(0); j<3; j++)
    {
      for(unsigned int k(0); k<3; k++)
      {
        for(unsigned int l(0); l<3; l++)
        {
          _vals[i][j][l][k] =
            _vals[j][i][k][l] = 
            _vals[j][i][l][k] =
            _vals[k][l][i][j] =
            _vals[l][k][j][i] = 
            _vals[k][l][j][i] =
            _vals[l][k][i][j] = _vals[i][j][k][l];
        }
      }
    }
  }
}

void
RankFourTensor::setValue(Real val, int i, int j, int k, int l)
{
  _vals[i-1][j-1][k-1][l-1] = val;
}

Real
RankFourTensor::getValue(int i, int j, int k, int l) const
{
  return _vals[i-1][j-1][k-1][l-1];
}

void
RankFourTensor::zero()
{
  for(unsigned int i(0); i<3; i++)
  {
    for(unsigned int j(0); j<3; j++)
    {
      for(unsigned int k(0); k<3; k++)
      {
        for(unsigned int l(0); l<3; l++)
        {
          _vals[i][j][k][l] = 0.0;
        }
      }
    }
  }
}


RankTwoTensor
RankFourTensor::operator*(const RankTwoTensor &a)
{
  RankTwoTensor result;

  Real t(0.0);
  
  for(unsigned int i(0); i<3; i++)
  {
    for(unsigned int j(0); j<3; j++)
    {
      t = 0.0;
      for(unsigned int k(0); k<3; k++)
      {
        for(unsigned int l(0); l<3; l++)
        {
          t += (_vals[i][j][k][l])*(a.getValue(k+1, l+1));
        }
      }
      //result[i][j] = t;
      result.setValue(t, i+1, j+1);
    }
  }
  return result;
}

RankFourTensor
RankFourTensor::operator*(const Real &a)
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      for(unsigned int k(0); k<3; k++)
        for(unsigned int l(0); l<3; l++)
          result.setValue(_vals[i][j][k][l]*a, i+1, j+1, k+1, l+1);

return result;
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor &a)
{
  for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      for(unsigned int k(0); k<3; k++)
        for(unsigned int l(0); l<3; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] + a.getValue(i+1, j+1, k+1, l+1);
  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor &a) const
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<3; i++)
    for(unsigned int j(0); j<3; j++)
      for(unsigned int k(0); k<3; k++)
        for(unsigned int l(0); l<3; l++)
          result.setValue(_vals[i][j][k][l]+a.getValue(i+1, j+1, k+1, l+1), i+1, j+1, k+1, l+1);

  return result;
}

/**
 * Deprecating this fugly mess.  Use elasticJacobian instead.
 */
Real
RankFourTensor::stiffness( const int i, const int j,
                           const RealGradient & test,
                           const RealGradient & phi)
{
  RealGradient b(0);
  
  int A[3], B[3], C[3], D[3];

  //these are actual crystal indices
  if (i == 0)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (i == 1)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (i == 2)
  {
    A[0] = 1; A[1] = 2; A[2] = 1;
    B[0] = 1; B[1] = 3; B[2] = 2;
  }
  if (j == 0)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }
  if (j == 1)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }
  if (j == 2)
  {
    C[0] = 1; C[1] = 2; C[2] = 1;
    D[0] = 1; D[1] = 3; D[2] = 2;
  }

  for(int k(0); k<3; k++)
  {
    for(int l(0); l<3; l++)
    {
      // array indices - thus subtracting 1
      int m, n, o, p;
      m = A[k] - 1;
      n = B[k] - 1;
      o = C[l] - 1;
      p = D[l] - 1;
      
      b(k) = b(k) + _vals[m][n][o][p]*phi(l);
    }
  }
  return test*b;
}

Real
RankFourTensor::elasticJacobian( const int i, const int k,
                      const RealGradient & grad_test,
                      const RealGradient & grad_phi)
{
  //Calculating Sum over (j,l) C_ijkl*grad_phi(k)[l]*grad_test(i)[j]
  Real a(0.0);

  for(int j(0); j<3; j++)
  {
    for (int l(0); l<3; l++)
    {
      a += _vals[i][j][k][l]*grad_phi(l)*grad_test(j);
    }
  }

  return a;
}

void
RankFourTensor::selfRotate(const Real a1, const Real a2, const Real a3)
{
  setFirstEulerAngle(a1);
  setSecondEulerAngle(a2);
  setThirdEulerAngle(a3);
  
  setRotationMatrix();

  Real temp;
  
  for(int i(0); i<3; i++)
  {
    for(int j(0); j<3; j++)
    {
      for(int k(0); k<3; k++)
      {
        for(int l(0); l<3; l++)
        {
          temp = 0.0;
          for(int m(0); m<3; m++)
          {
            for(int n(0); n<3; n++)
            {
              for(int o(0); o<3; o++)
              {
                for(int p(0); p<3; p++)
                {
                  temp += _rotation_matrix[i][m]*_rotation_matrix[j][n]
                    *_rotation_matrix[k][o]*_rotation_matrix[l][p]*_vals[m][n][o][p];
                }
              }
            }
          }
          _vals[i][j][k][l] = temp;
        }
      }
    }
  }
  //congratulations! you've reached the end of this endless loop.
}

RankFourTensor
RankFourTensor::rotate(const Real a1, const Real a2, const Real a3)
{
  RankFourTensor a;

  setFirstEulerAngle(a1);
  setSecondEulerAngle(a2);
  setThirdEulerAngle(a3);
  
  setRotationMatrix();

  Real temp;
  
  for(int i(0); i<3; i++)
  {
    for(int j(0); j<3; j++)
    {
      for(int k(0); k<3; k++)
      {
        for(int l(0); l<3; l++)
        {
          temp = 0.0;
          for(int m(0); m<3; m++)
          {
            for(int n(0); n<3; n++)
            {
              for(int o(0); o<3; o++)
              {
                for(int p(0); p<3; p++)
                {
                  temp += _rotation_matrix[i][m]*_rotation_matrix[j][n]
                    *_rotation_matrix[k][o]*_rotation_matrix[l][p]*_vals[m][n][o][p];
                }
              }
            }
          }
          a.setValue(temp, i+1, j+1, k+1, l+1);
        }
      }
    }
  }
  //congratulations! you've reached the end of this endless loop.
 return a;
}

void
RankFourTensor::setRotationMatrix()
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
RankFourTensor::setFirstEulerAngle(const Real a1)
{
  _euler_angle[0] = a1;
}

void
RankFourTensor::setSecondEulerAngle(const Real a2)
{
  _euler_angle[1] = a2;
}

void
RankFourTensor::setThirdEulerAngle(const Real a3)
{
  _euler_angle[2] = a3;
}

Real
RankFourTensor::firstEulerAngle() const
{
  return _euler_angle[0];
}

Real
RankFourTensor::secondEulerAngle() const
{
  return _euler_angle[1];
}

Real
RankFourTensor::thirdEulerAngle() const
{
  return _euler_angle[2];
}

RankFourTensor &
RankFourTensor::operator= (const RankFourTensor &a)
{
 _vals = a._vals;
 _euler_angle = a._euler_angle;
 _rotation_matrix = a._rotation_matrix;
 return *this; 
}
