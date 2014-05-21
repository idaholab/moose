#include "RankFourTensor.h"

// Any other includes here
#include <vector>
#include "libmesh/tensor_value.h"
#include "MaterialProperty.h"
#include "libmesh/libmesh.h"
#include <ostream>

extern "C" void FORTRAN_CALL(dsyev) ( ... );
extern "C" void FORTRAN_CALL(dgeev) ( ... );
extern "C" void FORTRAN_CALL(dgetri) ( ... );
extern "C" void FORTRAN_CALL(dgetrf) ( ... );


MooseEnum
RankFourTensor::fillMethodEnum()
{
  return MooseEnum("antisymmetric, symmetric9, symmetric21, general_isotropic, symmetric_isotropic, antisymmetric_isotropic, general");
}

RankFourTensor::RankFourTensor()
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
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
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = 0.0;
}

RankFourTensor &
RankFourTensor::operator= (const RankFourTensor &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = a._vals[i][j][k][l];

  return *this;
}

RankTwoTensor
RankFourTensor::operator*(const RankTwoTensor &a)
{
  RealTensorValue result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);

  return result;
}

RealTensorValue
RankFourTensor::operator*(const RealTensorValue &a)
{
  RealTensorValue result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);

  return result;
}

RankFourTensor
RankFourTensor::operator*(const Real &a)
{
  RankFourTensor result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]*a;

  return result;
}

RankFourTensor &
RankFourTensor::operator*=(const Real &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l]*a;

  return *this;
}

RankFourTensor
RankFourTensor::operator/(const Real &a)
{
  RankFourTensor result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]/a;

  return result;
}

RankFourTensor &
RankFourTensor::operator/=(const Real &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l]/a;

  return *this;
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] + a(i,j,k,l);

  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor &a) const
{
  RankFourTensor result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] + a(i,j,k,l);

  return result;
}

RankFourTensor &
RankFourTensor::operator-=(const RankFourTensor &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = _vals[i][j][k][l] - a(i,j,k,l);

  return *this;
}

RankFourTensor
RankFourTensor::operator-(const RankFourTensor &a) const
{
  RankFourTensor result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] - a(i,j,k,l);

  return result;
}

RankFourTensor
RankFourTensor::operator - () const
{
  RankFourTensor result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N;k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = -_vals[i][j][k][l];

  return result;
}

//Added
RankFourTensor
RankFourTensor::operator*(const RankFourTensor &a) const
{
  RankFourTensor result;


  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          for (unsigned int p(0); p<N; p++)
            for (unsigned int q(0); q<N; q++)
              result(i,j,k,l) += _vals[i][j][p][q]*a(p,q,k,l);

  return result;
}

RankFourTensor
RankFourTensor::invSymm()
{
  int error;
  double *mat;

  RankFourTensor result;

  unsigned int ntens=6;
  int nskip=2;

  mat=(double*)calloc(ntens*ntens, sizeof(double));


  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {

          if (i==j)
          {
            if (k==l)
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
            if (k==l)
            {
              mat[(nskip+i+j)*ntens+k]=_vals[i][j][k][l];
            }
            else
            {
              mat[(nskip+i+j)*ntens+nskip+k+l]+=_vals[i][j][k][l];
            }


          }
        }




  for (unsigned int i = 3; i < ntens; i++)
    for (unsigned int j = 3; j < ntens; j++)
      mat[i*ntens+j]=mat[i*ntens+j]/2.0;

  error=MatrixInversion(mat,ntens);

  if (error != 0)
    mooseError("Error in Matrix  Inversion in RankFourTensor\n");

  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
        {

          if (i==j)
          {
            if (k==l)
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
            if (k==l)
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

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
        {
          temp = 0.0;
          for (unsigned int m(0); m<N; m++)
            for (unsigned int n(0); n<N; n++)
              for (unsigned int o(0); o<N; o++)
                for (unsigned int p(0); p<N; p++)
                  temp += R(i,m)*R(j,n)*R(k,o)*R(l,p)*old._vals[m][n][o][p];

          _vals[i][j][k][l] = temp;
        }

}



void
RankFourTensor::print()
{
  RankFourTensor & s = (*this);

  for (unsigned int i=0; i<N; i++)
    for (unsigned int j=0; j<N; j++)
    {
      Moose::out << "i = " << i << " j = " << j << std::endl;
      for (unsigned int k=0; k<N; k++)
      {
        for (unsigned int l=0; l<N; l++)
          Moose::out << std::setw(15) <<s(i,j,k,l)<<" ";

        Moose::out <<std::endl;
      }
    }

}

RankFourTensor
RankFourTensor::transposeMajor()
{
  RankFourTensor result;

  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
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
  FORTRAN_CALL(dgetrf)(&n, &n, A, &n, ipiv, &return_value);

  if (return_value!=0)
  {

    free(ipiv);
    free(buffer);
    return return_value;
  }
  FORTRAN_CALL(dgetri)(&n, A, &n, ipiv, buffer, &buffer_size, &return_value);

  free(ipiv);
  free(buffer);
  return return_value;
}

void
RankFourTensor::surfaceFillFromInputVector(const std::vector<Real> input)
{
  if (input.size() == 9)
  {
    this->zero();

    //then fill from vector C_1111, C_1112, C_1122, C_1212, C_1222, C_1211, C_2211, C_2212, C_2222
    _vals[0][0][0][0]=input[0];
    _vals[0][0][0][1]=input[1];
    _vals[0][0][1][1]=input[2];
    _vals[0][1][0][1]=input[3];
    _vals[0][1][1][1]=input[4];
    _vals[0][1][0][0]=input[5];
    _vals[1][1][0][0]=input[6];
    _vals[1][1][0][1]=input[7];
    _vals[1][1][1][1]=input[8];

// any other symmetry to fill in?  just double-checking... e.g., C_1121?
    _vals[0][0][1][0]=_vals[0][0][0][1];
    _vals[0][1][1][0]=_vals[0][1][0][1];
    _vals[1][0][0][1]=_vals[0][1][0][1];
    _vals[1][0][1][1]=_vals[0][1][1][1];
    _vals[1][0][0][0]=_vals[0][1][0][0];
    _vals[1][1][1][0]=_vals[1][1][0][1];
  }
  else
    if (input.size() == 2)
    {
      this ->zero();
      // only two independent constants, C_1111 and C_1122
      _vals[0][0][0][0]=input[0];
      _vals[0][0][1][1]=input[1];
      //use symmetries
      _vals[1][1][1][1]=_vals[0][0][0][0];
      _vals[1][1][0][0]=_vals[0][0][1][1];
      _vals[0][1][0][1]=0.5*(_vals[0][0][0][0]-_vals[0][0][1][1]);
      _vals[1][0][0][1]=_vals[0][1][0][1];
      _vals[0][1][1][0]=_vals[0][1][0][1];
      _vals[1][0][1][0]=_vals[0][1][0][1];
    }
    else
      mooseError("Please provide correct number of inputs for surface RankFourTensor initialization.");
}

void
RankFourTensor::fillSymmetricFromInputVector(const std::vector<Real> input, bool all)
{
  if ((all == true && input.size() != 21) || (all == false && input.size() != 9 ))
    mooseError("Please check the number of entries in the stiffness input vector.");

  zero();

  if (all == true)
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
    _vals[0][1][0][2] = input[19]; //C1312 //flipped for filling purposesc

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
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][l][k] =
            _vals[j][i][k][l] =
            _vals[j][i][l][k] =
            _vals[k][l][i][j] =
            _vals[l][k][j][i] =
            _vals[k][l][j][i] =
            _vals[l][k][i][j] = _vals[i][j][k][l];
}



void
RankFourTensor::fillAntisymmetricFromInputVector(const std::vector<Real> input)
{
  if (input.size() != 6)
    mooseError("To use fillAntisymmetricFromInputVector, your input must have size 6.  Yours has size " << input.size());

  zero();

  _vals[0][1][0][1] = input[0];   //B1212
  _vals[0][1][0][2] = input[1];   //B1213
  _vals[0][1][1][2] = input[2];   //B1223

  _vals[0][2][0][2] = input[3];   //B1313
  _vals[0][2][1][2] = input[4];   //B1323

  _vals[1][2][1][2] = input[5];   //B2323

  // antisymmetry
  _vals[0][2][0][1] = -_vals[0][1][0][2];
  _vals[1][2][0][1] = -_vals[0][1][1][2];
  _vals[1][2][0][2] = -_vals[0][2][1][2];

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N - 1 ; i++)
    for (unsigned int j = i + 1 ; j < N ; j++)
    {
      _vals[0][1][j][i] = -_vals[0][1][i][j];
      _vals[0][2][j][i] = -_vals[0][2][i][j];
      _vals[1][2][j][i] = -_vals[1][2][i][j];
    }

  // fill in from symmetry relations
  for (unsigned int i = 0; i < N - 1 ; i++)
    for (unsigned int j = i + 1 ; j < N ; j++)
    {
      _vals[i][j][0][1] = _vals[0][1][i][j];
      _vals[i][j][0][2] = _vals[0][2][i][j];
      _vals[i][j][1][2] = _vals[1][2][i][j];
    }
}


void
RankFourTensor::fillGeneralIsotropicFromInputVector(const std::vector<Real> input)
{
  if (input.size() != 3)
    mooseError("To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours has size " << input.size());

  zero();

  for (unsigned int i=0; i<N; i++)
    for (unsigned int j=0; j<N; j++)
      for (unsigned int k=0; k<N; k++)
        for (unsigned int l=0; l<N; l++)
        {
          _vals[i][j][k][l] = input[0]*(i==j)*(k==l) + input[1]*(i==k)*(j==l) + input[1]*(i==l)*(j==k);
            for (unsigned int m = 0 ; m < N ; m++)
              _vals[i][j][k][l] += input[2]*PermutationTensor::eps(i, j, m)*PermutationTensor::eps(k, l, m);
        }
}

void
RankFourTensor::fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1.  Yours has size " << input.size());
  std::vector<Real> input3;
  input3.push_back(0);
  input3.push_back(0);
  input3.push_back(input[0]);
  fillGeneralIsotropicFromInputVector(input3);
}

void
RankFourTensor::fillSymmetricIsotropicFromInputVector(const std::vector<Real> input)
{
  if (input.size() != 2)
    mooseError("To use fillSymmetricIsotropicFromInputVector, your input must have size 2.  Yours has size " << input.size());
  std::vector<Real> input3;
  input3.push_back(input[0]);
  input3.push_back(input[1]);
  input3.push_back(0);
  fillGeneralIsotropicFromInputVector(input3);
}


void
RankFourTensor::fillGeneralFromInputVector(const std::vector<Real> input)
{
  if (input.size() != 81)
    mooseError("To use fillGeneralFromInputVector, your input must have size 81.  Yours has size " << input.size());

  zero();

  int ind;
  for (unsigned int i=0; i<N; i++)
    for (unsigned int j=0; j<N; j++)
      for (unsigned int k=0; k<N; k++)
        for (unsigned int l=0; l<N; l++)
        {
          ind = i*N*N*N + j*N*N + k*N + l;
          _vals[i][j][k][l] = input[ind];
        }
}

void
RankFourTensor::fillFromInputVector(const std::vector<Real> input, FillMethod fill_method)
{
  zero();

  switch(fill_method)
  {
    case antisymmetric:
      fillAntisymmetricFromInputVector(input);
      break;
    case symmetric9:
      fillSymmetricFromInputVector(input, false);
      break;
    case symmetric21:
      fillSymmetricFromInputVector(input, true);
      break;
    case general_isotropic:
      fillGeneralIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic:
      fillSymmetricIsotropicFromInputVector(input);
      break;
    case antisymmetric_isotropic:
      fillAntisymmetricIsotropicFromInputVector(input);
      break;
    case general:
      fillGeneralFromInputVector(input);
      break;
    default:
      mooseError("fillFromInputVector called with unknown fill_method of " << fill_method);
  }
}
