#ifndef RANKTWOEIGENROUTINES_H
#define RANKTWOEIGENROUTINES_H

#include "RankFourTensor.h"

namespace RankTwoEigenRoutines
{
  static const unsigned int N = 3;

  /**
   * Computes second derivatives of Eigenvalues of a rank two tensor
   * @param tens is a rank two tensor
   * @param deriv is a second derivative of the input tensor
   */
  void d2symmetricEigenvalues(RankTwoTensor & tens, std::vector<RankFourTensor> & deriv);

  /**
   * computes eigenvalues, assuming tens is symmetric, and places them
   * in ascending order in eigvals
   */
  void symmetricEigenvalues(RankTwoTensor & tens, std::vector<Real> & eigvals);

  /**
   * computes eigenvalues, and their symmetric derivatives wrt vals,
   * assuming tens is symmetric
   * @param eigvals are the eigenvalues of the matrix, in ascending order
   * @param deigvals Here digvals[i](j,k) = (1/2)*(d(eigvals[i])/dA_jk + d(eigvals[i]/dA_kj))
   * Note the explicit symmeterisation here.
   * For equal eigenvalues, these derivatives are not gauranteed to
   * be the ones you expect, since the derivatives in this case are
   * often defined by continuation from the un-equal case, and that is
   * too sophisticated for this routine.
   */
  void dsymmetricEigenvalues(RankTwoTensor & tens, std::vector<Real> & eigvals, std::vector<RankTwoTensor> & deigvals);
   /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to find, for symmetric _vals:
   *  (1) the eigenvalues (if calculation_type == "N")
   *  (2) the eigenvalues and eigenvectors (if calculation_type == "V")
   * @param calculation_type If "N" then calculation eigenvalues only
   * @param eigvals Eigenvalues are placed in this array, in ascending order
   * @param a Eigenvectors are placed in this array if calculation_type == "V".
   * See code in dsymmetricEigenvalues for extracting eigenvectors from the a output.
   */
  void syev(RankTwoTensor & tens, const char * calculation_type, std::vector<Real> & eigvals, std::vector<double> & a);

}
#endif //RANKTWOEIGENROUTINES_H
