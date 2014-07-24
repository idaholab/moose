#ifndef RETURNMAPUTILS_H
#define RETURNMAPUTILS_H

// Modules includes
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/**
 * Utility functions that may be called during
 * the return-map of TensorMechanics plasticity
 * routines
 */
namespace ReturnMapUtils
{
  /** Solve the linear system
   *
   * ( ddirn_dstress ddirn_dpm ddirn_dintnl )( dstress )    ( dirn )
   * (  df_dstress       0      df_dintnl   )(   dpm   ) = -(  f   )
   * ( dic_dstress    dic_dpm   dic_dintnl  )( dintnl  )    (  ic  )
   *
   * for dstress, dpm and dintnl, assuming that dstress and related
   * tensor quantities are symmetric
   *
   * @param dirn the "direction constraint" from plasticity
   * @param f vector of yield functions
   * @param ic vector of internal constraints from plasticity
   */
  void linearSolve(const RankTwoTensor & dirn, const std::vector<Real> & f, const std::vector<Real> & ic, const RankFourTensor & ddirn_dstress, const std::vector<RankTwoTensor> & ddirn_dpm, const std::vector<RankTwoTensor> & ddirn_dintnl, const std::vector<RankTwoTensor> & df_dstress, const std::vector<std::vector<Real> > & df_dintnl, const std::vector<RankTwoTensor> & dic_dstress, const std::vector<std::vector<Real> > & dic_dpm, const std::vector<std::vector<Real> > & dic_dintnl, RankTwoTensor & dstress, std::vector<Real> & dpm, std::vector<Real> & dintnl);

  /// print routine for RHS in linearSolve (useful for debugging)
  void printRHS(const std::vector<double> & rhs);

  /// print routine for square matrix A in linearSolve (useful for debugging)
  void printA(const std::vector<double> & a);


  Real solutionError(const RankTwoTensor & dirn, const std::vector<Real> & f, const std::vector<Real> & ic, const RankFourTensor & ddirn_dstress, const std::vector<RankTwoTensor> & ddirn_dpm, const std::vector<RankTwoTensor> & ddirn_dintnl, const std::vector<RankTwoTensor> & df_dstress, const std::vector<std::vector<Real> > & df_dintnl, const std::vector<RankTwoTensor> & dic_dstress, const std::vector<std::vector<Real> > & dic_dpm, const std::vector<std::vector<Real> > & dic_dintnl, RankTwoTensor & dstress, const std::vector<Real> & dpm, const std::vector<Real> & dintnl);
}

#endif //RETURNMAPUTILS_H
