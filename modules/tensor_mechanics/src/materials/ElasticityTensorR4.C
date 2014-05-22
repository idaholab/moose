#include "ElasticityTensorR4.h"
#include "MaterialProperty.h"


Real
ElasticityTensorR4::elasticJacobian( const unsigned int i, const unsigned int k,
                                     const RealGradient & grad_test,
                                     const RealGradient & grad_phi)
{
  //Calculating Sum over (j,l) C_ijkl*grad_phi(k)[l]*grad_test(i)[j]
  Real a(0.0);

  for (unsigned int j(0); j<N; j++)
    for (unsigned int l(0); l<N; l++)
      a += _vals[i][j][k][l]*grad_phi(l)*grad_test(j);
  //a += _vals[i][j][l][k]*grad_phi(l)*grad_test(j);

  return a;
}

Real
ElasticityTensorR4::cosseratElasticJacobian( const unsigned int i, const unsigned int k,
                                     const RealGradient & grad_test,
                                     const Real & phi)
{
  Real a(0.0);

  for (unsigned int j(0); j<N; j++)
    for (unsigned int m(0); m<N; m++)
      for (unsigned int n(0); n<N; n++)
        a += _vals[i][j][m][n]*PermutationTensor::eps(m, n, k)*grad_test(j);
  return a*phi;
}

Real
ElasticityTensorR4::momentJacobian( const unsigned int comp1, const unsigned int comp2, const Real & test,
                                     const RealGradient & grad_phi)
{
  Real the_sum = 0;
  for (unsigned int i = 0 ; i < N ; ++i)
    for (unsigned int j = 0 ; j < N ; ++j)
      for (unsigned int k = 0 ; k < N ; ++k)
        //the_sum += _vals[i][j][k][comp2]*grad_phi(k)*PermutationTensor::eps(i, j, comp1);
        the_sum += _vals[j][i][comp2][k]*grad_phi(k)*PermutationTensor::eps(i, j, comp1);
  return -test*the_sum;
}

Real
ElasticityTensorR4::momentJacobianwc( const unsigned int comp1, const unsigned int comp2, const Real & test,
                                     const Real & phi)
{
  Real the_sum = 0;
  for (unsigned int j = 0 ; j < N ; ++j)
    for (unsigned int k = 0 ; k < N ; ++k)
      for (unsigned int m = 0 ; m < N ; ++m)
        for (unsigned int n = 0 ; n < N ; ++n)
          the_sum += _vals[j][k][m][n]*PermutationTensor::eps(comp1, j, k)*PermutationTensor::eps(m, n, comp2);
  return test*phi*the_sum;
}

ElasticityTensorR4&
ElasticityTensorR4::operator=(const ElasticityTensorR4 &a)
{
  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          _vals[i][j][k][l] = a._vals[i][j][k][l];

  return *this;
}

ElasticityTensorR4
ElasticityTensorR4::operator/(const Real &a)
{
  ElasticityTensorR4 result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]/a;

  return result;
}

ElasticityTensorR4
ElasticityTensorR4::operator+(const ElasticityTensorR4 &a) const
{
  ElasticityTensorR4 result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] + a(i,j,k,l);

  return result;
}

ElasticityTensorR4
ElasticityTensorR4::operator-(const ElasticityTensorR4 &a) const
{
  ElasticityTensorR4 result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l] - a(i,j,k,l);

  return result;
}

ElasticityTensorR4
ElasticityTensorR4::operator - () const
{
  ElasticityTensorR4 result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = -_vals[i][j][k][l];

  return result;
}

ElasticityTensorR4
ElasticityTensorR4::operator*(const Real &a)
{
  ElasticityTensorR4 result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j,k,l) = _vals[i][j][k][l]*a;

  return result;
}

RankTwoTensor
ElasticityTensorR4::operator*(const RankTwoTensor &a)
{
  RealTensorValue result;

  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          result(i,j) += _vals[i][j][k][l]*a(k,l);

  return result;
}

ElasticityTensorR4
ElasticityTensorR4::operator*(const RankFourTensor &a) const
{
  ElasticityTensorR4 result;


  for (unsigned int i(0); i<N; i++)
    for (unsigned int j(0); j<N; j++)
      for (unsigned int k(0); k<N; k++)
        for (unsigned int l(0); l<N; l++)
          for (unsigned int p(0); p<N; p++)
            for (unsigned int q(0); q<N; q++)
              result(i,j,k,l) += _vals[i][j][p][q]*a(p,q,k,l);

  return result;
}
