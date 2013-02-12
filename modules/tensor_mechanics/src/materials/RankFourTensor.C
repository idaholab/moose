#include "RankFourTensor.h"

// Any other includes here
#include <vector>
#include "libmesh/tensor_value.h"
#include "MaterialProperty.h"
#include "libmesh/libmesh.h"
#include <ostream>

extern "C" void dsyev_ ( ... );
extern "C" void dgeev_ ( ... );
extern "C" void dgetri_ ( ... );
extern "C" void dgetrf_ ( ... );
     

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

//Added
RankFourTensor
RankFourTensor::operator*(const RankFourTensor &a) const
{
  RankFourTensor result;
  
  
  for(unsigned int i(0); i<N; i++)
    for(unsigned int j(0); j<N; j++)
      for(unsigned int k(0); k<N; k++)
        for(unsigned int l(0); l<N; l++)
          for(unsigned int p(0); p<N; p++)
            for(unsigned int q(0); q<N; q++)
              result(i,j,k,l) += _vals[i][j][p][q]*a(p,q,k,l);  

  return result;
}

RankFourTensor
RankFourTensor::invSymm()
{
  int nsize,error;
  double *mat;
  
  RankFourTensor result;

  int ntens=6;
  int nskip=2;
  
  mat=(double*)calloc(ntens*ntens, sizeof(double));


  for(unsigned int i = 0; i < 3; i++)
    for(unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for(unsigned int l = 0; l < 3; l++)
        {

          if(i==j)
          {
            if(k==l)
            {
              mat[i*ntens+k]=_vals[i][j][k][l];
            }
            else
            {
              mat[i*ntens+nskip+k+l]+=_vals[i][j][k][l];
            }
            
          }
          else
          {
            if(k==l)
            {
              mat[(nskip+i+j)*ntens+k]=_vals[i][j][k][l];
            }
            else
            {
              mat[(nskip+i+j)*ntens+nskip+k+l]+=_vals[i][j][k][l];
            }

            
          }
        }


  
  
  for(unsigned int i = 3; i < ntens; i++)
    for(unsigned int j = 3; j < ntens; j++)
      mat[i*ntens+j]=mat[i*ntens+j]/2.0;
  
  error=MatrixInversion(mat,ntens);

  if(error != 0)
    mooseError("Error in Matrix  Inversion in RankFourTensor\n");

  for(unsigned int i = 0; i < 3; i++)
    for(unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for(unsigned int l = 0; l < 3; l++)
        {

          if(i==j)
          {
            if(k==l)
            {
              result(i,j,k,l)=mat[i*ntens+k];             
              
            }
            else
            {

              result(i,j,k,l)=mat[i*ntens+2+k+l]/2.0;
              
            }
            
          }
          else
          {
            if(k==l)
            {
              result(i,j,k,l)=mat[(2+i+j)*ntens+k]; 
              
            }
            else
            {

              result(i,j,k,l)=mat[(2+i+j)*ntens+2+k+l]/2.0;             

            }

            
          }
        }

  free(mat);
  return result;
  
  
}


//

void
RankFourTensor::rotate(RealTensorValue &R)
{
  Real temp;
  RankFourTensor old;
  old=*this;
  
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
                  temp += R(i,m)*R(j,n)*R(k,o)*R(l,p)*old._vals[m][n][o][p];
          
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

RankFourTensor
RankFourTensor::transposeMajor()
{
  RankFourTensor result;

  for(unsigned int i = 0; i < 3; i++)
    for(unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for(unsigned int l = 0; l < 3; l++)
          result(i,j,k,l)=_vals[k][l][i][j];


  return result;
  
}





int
RankFourTensor::MatrixInversion(double* A, int n)
{
  int return_value,buffer_size;
  int *ipiv,*buffer; 
  
  buffer_size=n*64;

  ipiv=(int*)calloc(n, sizeof(int));
  buffer=(int*)calloc(buffer_size, sizeof(int));  
  dgetrf_(&n, &n, A, &n, ipiv, &return_value);
  
  if (return_value!=0)
  {
    
    free(ipiv);
    free(buffer);   
    return return_value;
  }  
  dgetri_(&n, A, &n, ipiv, buffer, &buffer_size, &return_value);
  
  free(ipiv);
  free(buffer);   
  return return_value;
  

}



