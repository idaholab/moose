#include "RankFourTensor.h"

// Any other includes here
#include <vector>
#include "tensor_value.h"
#include "MaterialProperty.h"
#include "libmesh.h"
#include <ostream>


RankFourTensor::RankFourTensor() 
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensor::RankFourTensor(const RankFourTensor &a)
{
  *this = a;
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
RankFourTensor::setValue(Real val, unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  _vals[i-1][j-1][k-1][l-1] = val; //Note, indcies go from 1 to 3
}

Real
RankFourTensor::getValue(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  return _vals[i-1][j-1][k-1][l-1];//Note, indcies go from 1 to 3
}

void
RankFourTensor::zero()
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensor &
RankFourTensor::operator= (const RankFourTensor &a)
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = a._vals[i][j][k][l];
  
  return *this; 
}

RealTensorValue
RankFourTensor::operator*(const RankTwoTensor &a)
{
  RealTensorValue result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);
  
  return result;
}

RealTensorValue
RankFourTensor::operator*(const RealTensorValue &a)
{
  RealTensorValue result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);
  
  return result;
}

RankFourTensor
RankFourTensor::operator*(const Real &a)
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]*a;

return result;
}

RankFourTensor &
RankFourTensor::operator*=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l]*a;
  
  return *this;
}
RankFourTensor
RankFourTensor::operator/(const Real &a)
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]/a;

return result;
}

RankFourTensor &
RankFourTensor::operator/=(const Real &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l]/a;
  
  return *this;
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] + a(i,j,k,l);
  
  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor &a) const
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] + a(i,j,k,l);

  return result;
}

RankFourTensor &
RankFourTensor::operator-=(const RankFourTensor &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] - a(i,j,k,l);
  
  return *this;
}

RankFourTensor
RankFourTensor::operator-(const RankFourTensor &a) const
{
  RankFourTensor result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] - a(i,j,k,l);

  return result;
}

RankFourTensor
RankFourTensor::operator - () const
{
  RankFourTensor result;

  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = -_vals[i][j][k][l];

  return result;
}


void
RankFourTensor::rotate(RealTensorValue &R)
{
  Real temp;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
        {
          temp = 0.0;
          for(unsigned int m(0); m<N; m++)
            for(unsigned int n(0); n<N; n++)
              for(unsigned int o(0); o<N; o++)
                for(unsigned int p(0); p<N; p++)
                  temp += R(i,m)*R(j,n)*R(k,o)*R(l,p)*_vals[m][n][o][p];
          
          _vals[i][j][k][l] = temp;
        }
  
                
  std::cout << std::endl;
  
  
}

void
RankFourTensor::print()
{
  RankFourTensor & s = (*this);

  for(unsigned int i=0; i<N; i++)
    for(unsigned int j=0; j<N; j++)
    {
      std::cout << "i = " << i << " j = " << j << std::endl;
      for(unsigned int k=0; k<N; k++)
      {
        for(unsigned int l=0; l<N; l++)
          std::cout << std::setw(15) <<s(i,j,k,l)<<" ";
        
        std::cout <<std::endl;
      }
    }
  
}
