#include "RankFourTensorTonks.h"

// Any other includes here
#include <vector>
#include "tensor_value.h"
#include "MaterialProperty.h"
#include "libmesh.h"
#include <ostream>


RankFourTensorTonks::RankFourTensorTonks() 
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensorTonks::RankFourTensorTonks(const RankFourTensorTonks &a)
{
  *this = a;
}

Real &
RankFourTensorTonks::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  return _vals[i][j][k][l];
}

Real 
RankFourTensorTonks::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  return _vals[i][j][k][l];
}

void
RankFourTensorTonks::zero()
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensorTonks &
RankFourTensorTonks::operator= (const RankFourTensorTonks &a)
{
  for (unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = a._vals[i][j][k][l];
  
  return *this; 
}

RealTensorValue
RankFourTensorTonks::operator*(const RealTensorValue &a)
{
  RealTensorValue result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);
  
  return result;
}

RankFourTensorTonks
RankFourTensorTonks::operator*(const Real &a)
{
  RankFourTensorTonks result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]*a;

return result;
}

RankFourTensorTonks &
RankFourTensorTonks::operator+=(const RankFourTensorTonks &a)
{
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] + a(i,j,k,l);
  
  return *this;
}

RankFourTensorTonks
RankFourTensorTonks::operator+(const RankFourTensorTonks &a) const
{
  RankFourTensorTonks result;
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] + a(i,j,k,l);

  return result;
}


void
RankFourTensorTonks::rotate(RealTensorValue &R)
{  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          for(unsigned int m(0); m<N; m++)
            for(unsigned int n(0); n<N; n++)
              for(unsigned int o(0); o<N; o++)
                for(unsigned int p(0); p<N; p++)
                {
                  //std::cout << _vals[i][j][k][l];
                  _vals[i][j][k][l] = R(i,m)*R(j,n)*R(k,o)*R(l,p)*_vals[m][n][o][p];
                }
  std::cout << std::endl;
  
  
}

void
RankFourTensorTonks::print()
{
  RankFourTensorTonks & s = (*this);

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
