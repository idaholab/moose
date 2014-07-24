#include "ReturnMapUtils.h"

// Following are used to access PETSc's LAPACK routines
#include "MaterialProperty.h"
#include <petscblaslapack.h>

void
ReturnMapUtils::linearSolve(const RankTwoTensor & dirn, const std::vector<Real> & f, const std::vector<Real> & ic, const RankFourTensor & ddirn_dstress, const std::vector<RankTwoTensor> & ddirn_dpm, const std::vector<RankTwoTensor> & ddirn_dintnl, const std::vector<RankTwoTensor> & df_dstress, const std::vector<std::vector<Real> > & df_dintnl, const std::vector<RankTwoTensor> & dic_dstress, const std::vector<std::vector<Real> > & dic_dpm, const std::vector<std::vector<Real> > & dic_dintnl, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl)
{
  unsigned int dim = 3;
  unsigned int num_f = f.size(); // number of yield functions, and the number of pm
  unsigned int num_ic = ic.size();
  int system_size = dim*(dim+1)/2 + num_f + num_ic;


  /**
   * This is a nasty piece of code.  The idea is
   * simple: just solve A x = rhs for x.  But filling
   * the "A" and "rhs" in preparation for the LAPACKgesv_
   * is pretty hairy.
   * Extra hairiness is added because the full "A" is
   * often not invertible because of the explicit
   * symmeterisation of stresses and strains, so I
   * explicitly enforce the constraint
   * dstress(i, j) = dstress(j, i)
   * by solving for dstress(i, j) with i>=j
   */

  /**
   * In the following the indices:
   *    i, j, k and l will run from 0 to dim-1 (they are tensor indices)
   *    fi and fj will run from 0 to num_f-1 (runs over the yield functions and/or plastic multipliers)
   *    ci and cj will run from 0 to num_ic-1 (runs over the internal constraints)
   */


  /**
   * Create rhs and fill
   * rhs = -(dirn(0,0), dirn(1,0), dirn(1,1), dirn(2,0), dirn(2,1), dirn(2,2), f[0], f[1], ..., f[num_f], ic[0], ic[1], ..., ic[num_ic])
   * notice the appearance of only the i>=j components
   */
  std::vector<double> rhs(system_size);
  unsigned int ind = 0;
  for (unsigned i = 0 ; i < dim ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
      rhs[ind++] = -dirn(i, j);
  for (unsigned fi = 0 ; fi < num_f ; ++fi)
    rhs[ind++] = -f[fi];
  for (unsigned ci = 0 ; ci < num_ic ; ++ci)
    rhs[ind++] = -ic[ci];

  //printRHS(rhs);


  /**
   * In block form, the matrix is
   *
   * ( ddirn_dstress ddirn_dpm ddirn_dintnl )
   * (  df_dstress       0      df_dintnl   )
   * ( dic_dstress    dic_dpm   dic_dintnl  )
   *
   */

  /**
   * Example of conversion from a matrix to "a":
   *
   *     ( m00 m01 m02 )   ( a0 a3 a6 )
   * m = ( m10 m11 m12 ) = ( a1 a4 a7 )
   *     ( m20 m21 m22 )   ( a2 a5 a8 )
   *
   * so a[i + j*size] = m[i][j]
   */
  std::vector<double> a(system_size*system_size);

  ind = 0;

  // Fill in the blocks by going down columns
  for (unsigned k = 0 ; k < dim ; ++k)
    for (unsigned l = 0 ; l <= k ; ++l)
    {
      for (unsigned i = 0 ; i < dim ; ++i)
        for (unsigned j = 0 ; j <= i ; ++j)
          a[ind++] = ddirn_dstress(i, j, k, l) + (k != l ? ddirn_dstress(i, j, l, k) : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i) - see note above

      for (unsigned fi = 0 ; fi < num_f ; ++fi)
        a[ind++] = df_dstress[fi](k, l) + (k != l ? df_dstress[fi](l, k) : 0);

      for (unsigned ci = 0 ; ci < num_ic ; ++ci)
        a[ind++] = dic_dstress[ci](k, l) + (k != l ? dic_dstress[ci](l, k) : 0);
    }

  for (unsigned fj = 0 ; fj < num_f ; ++fj)
    {
      for (unsigned i = 0 ; i < dim ; ++i)
        for (unsigned j = 0 ; j <= i ; ++j)
          a[ind++] = ddirn_dpm[fj](i, j);

      for (unsigned fi = 0 ; fi < num_f ; ++fi)
        a[ind++] = 0; // This is df[fi]/dpm[fj], but f never depends on plasticity multipliers

      for (unsigned ci = 0 ; ci < num_ic ; ++ci)
        a[ind++] = dic_dpm[ci][fj];
    }


  for (unsigned cj = 0 ; cj < num_ic ; ++cj)
    {
      for (unsigned i = 0 ; i < dim ; ++i)
        for (unsigned j = 0 ; j <= i ; ++j)
          a[ind++] = ddirn_dintnl[cj](i, j);

      for (unsigned fi = 0 ; fi < num_f ; ++fi)
        a[ind++] = df_dintnl[fi][cj];

      for (unsigned ci = 0 ; ci < num_ic ; ++ci)
        a[ind++] = dic_dintnl[ci][cj];
    }


  //printA(a);

  // solve using PETSc's LAPACK wrapper
  int nrhs = 1;
  std::vector<int> ipiv(system_size);
  int info;
  LAPACKgesv_(&system_size, &nrhs, &a[0], &system_size, &ipiv[0], &rhs[0], &system_size, &info);

  if (info != 0)
    mooseError("In solving the linear system in a Newton-Raphson process, the PETSC LAPACK gsev routine returned with error code " << info);


  // Extract the results back to dstress, dpm and dintnl
  ind = 0;
  for (unsigned i = 0 ; i < dim ; ++i)
    for (unsigned j = 0 ; j <= i ; ++j)
      dstress(i, j) = dstress(j, i) = rhs[ind++];
  dpm.resize(num_f);
  for (unsigned fi = 0 ; fi < num_f ; ++fi)
    dpm[fi] = rhs[ind++];
  dintnl.resize(num_ic);
  for (unsigned ci = 0 ; ci < num_ic ; ++ci)
    dintnl[ci] = rhs[ind++];
}



void
ReturnMapUtils::printRHS(const std::vector<double> & rhs)
{
  Moose::out << "rhs = ";
  for (unsigned i = 0; i< rhs.size(); ++i)
    Moose::out <<  rhs[i] << " ";
  Moose::out << "\n";
}

void
ReturnMapUtils::printA(const std::vector<double> & a)
{
  Moose::out << "a = \n";
  int n = (int) (std::sqrt(a.size()) + 0.5); // 0.5 is here because of round-down
  for (int row = 0 ; row < n ; ++row)
  {
    for (int col = 0 ; col < n ; ++col)
      Moose::out << a[row + col*n] << " ";
    Moose::out << "\n";
  }
  /*
  Moose::out << "{";
  for (int row = 0 ; row < n ; ++row)
  {
    Moose::out << "{";
    for (int col = 0 ; col < n - 1 ; ++col)
      Moose::out << a[row + col*n] << ", ";
    Moose::out << a[row + (n-1)*n] << "},";
  }
  Moose::out << "}\n";
  */

}



Real
ReturnMapUtils::solutionError(const RankTwoTensor & dirn, const std::vector<Real> & f, const std::vector<Real> & ic, const RankFourTensor & ddirn_dstress, const std::vector<RankTwoTensor> & ddirn_dpm, const std::vector<RankTwoTensor> & ddirn_dintnl, const std::vector<RankTwoTensor> & df_dstress, const std::vector<std::vector<Real> > & df_dintnl, const std::vector<RankTwoTensor> & dic_dstress, const std::vector<std::vector<Real> > & dic_dpm, const std::vector<std::vector<Real> > & dic_dintnl, RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl)
{
  unsigned int num_f = f.size(); // number of yield functions, and the number of pm
  unsigned int num_ic = ic.size();


  RankTwoTensor r = ddirn_dstress*dstress;
  for (unsigned i = 0 ; i < num_f ; ++i)
    r += ddirn_dpm[i]*dpm[i];
  for (unsigned i = 0 ; i < num_ic ; ++i)
    r += ddirn_dintnl[i]*dintnl[i];
  r += dirn;

  Real error = r.L2norm();
  error *= error; // i'll add the square of the other values too

  std::vector<Real> fe(num_f);
  for (unsigned i = 0 ; i < num_f ; ++i)
    fe[i] = dstress.doubleContraction(df_dstress[i]);
  for (unsigned i = 0 ; i < num_f ; ++i)
    for (unsigned j = 0 ; j < num_ic ; ++j)
      fe[i] += df_dintnl[i][j]*dintnl[j];
  for (unsigned i = 0 ; i < num_f ; ++i)
    error += std::pow(fe[i] + f[i], 2);

  std::vector<Real> ice(num_ic);
  for (unsigned i = 0 ; i < num_ic ; ++i)
    ice[i] = dstress.doubleContraction(dic_dstress[i]);
  for (unsigned i = 0 ; i < num_ic ; ++i)
    for (unsigned j = 0 ; j < num_f ; ++j)
      ice[i] += dic_dpm[i][j]*dpm[j];
  for (unsigned i = 0 ; i < num_ic ; ++i)
    for (unsigned j = 0 ; j < num_ic ; ++j)
      ice[i] += dic_dintnl[i][j]*dintnl[j];
  for (unsigned i = 0 ; i < num_ic ; ++i)
    error += std::pow(ice[i] + ic[i], 2);


  return error;

}

