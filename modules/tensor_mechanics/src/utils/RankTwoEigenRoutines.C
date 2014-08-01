#include "RankTwoEigenRoutines.h"
#include "MaterialProperty.h"
// This is used to calculate eigenvalues and eigenvectors
#include <petscblaslapack.h>
namespace RankTwoEigenRoutines
{
  void
  symmetricEigenvalues(RankTwoTensor & tens, std::vector<Real> & eigvals)
  {
    std::vector<double> a;
    syev(tens, "N", eigvals, a);
  }

  void
  d2symmetricEigenvalues(RankTwoTensor & tens, std::vector<RankFourTensor> & deriv)
  {
    std::vector<double> eigvec;
    std::vector<double> eigvals;
    Real ev[N][N];

    // reset rank four tensor
    deriv.assign(N, RankFourTensor());

    // get eigen values and eigen vectors
    syev(tens, "V",eigvals,eigvec);

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        ev[i][j] = eigvec[i*N + j];

    for (unsigned int alpha = 0; alpha < N; alpha++)
      for (unsigned int beta = 0; beta < N; beta++)
      {
        if (eigvals[alpha] == eigvals[beta])
          continue;
        for (unsigned int i = 0; i < N; i++)
          for (unsigned int j = 0; j < N; j++)
            for (unsigned int k = 0; k < N; k++)
              for (unsigned int l = 0; l < N; l++)
              {
                deriv[alpha](i, j, k, l) += 0.5*(ev[beta][i]*ev[alpha][j]+ev[alpha][i]*ev[beta][j])
                  *(ev[beta][k]*ev[alpha][l]+ev[beta][l]*ev[alpha][k])/(eigvals[alpha]-eigvals[beta]);
              }
      }
  }

  void
  dsymmetricEigenvalues(RankTwoTensor & tens, std::vector<Real> & eigvals, std::vector<RankTwoTensor> & deigvals)
  {
    deigvals.resize(N);

    std::vector<double> a;
    syev(tens, "V", eigvals, a);

    // now a contains the eigenvetors
    // extract these and place appropriately in deigvals
    std::vector<Real> eig_vec;
    eig_vec.resize(N);

    for (unsigned int i = 0; i < N; ++i)
    {
      for (unsigned int j = 0; j < N; ++j)
        eig_vec[j] = a[i*N + j];
      for (unsigned int j = 0; j < N; ++j)
        for (unsigned int k = 0; k < N; ++k)
          deigvals[i](j, k) = eig_vec[j]*eig_vec[k];
    }
  }

  void
  syev(RankTwoTensor & tens, const char * calculation_type, std::vector<Real> & eigvals, std::vector<double> & a)
  {
    eigvals.resize(N);
    a.resize(N*N);

    // prepare data for the LAPACKsyev_ routine (which comes from petscblaslapack.h)
    int nd = N;
    int lwork = 66*nd;
    int info;
    std::vector<double> work(lwork);

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        a[i*N + j] = tens.operator()(i,j); // a is destroyed by dsyev, and if calculation_type == "V" then eigenvectors are placed there

    // compute the eigenvalues only (if calculation_type == "N"),
    // or both the eigenvalues and eigenvectors (if calculation_type == "V")
    // assume upper triangle of a is stored (second "U")
    LAPACKsyev_(calculation_type, "U", &nd, &a[0], &nd, &eigvals[0], &work[0], &lwork, &info);

    if (info != 0)
      mooseError("In computing the eigenvalues and eigenvectors of a symmetric rank-2 tensor, the PETSC LAPACK syev routine returned error code " << info);
  }

}
