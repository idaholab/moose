/**
 * ElasticityTensorR4 is designed as a general elastiicty tensor that can handle
 * any symmetry.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * ElasticityTensorR4 holds the 81 separate C_ijkl entries; the entries are accessed by index, with
 * i, j, k, and l equal to 0, 1, 2
 * 
 */

#ifndef ELASTICITYTENSORR4_H
#define ELASTICITYTENSORR4_H

// Any requisite includes here
#include "RankFourTensorTonks.h"

class ElasticityTensorR4 : public RankFourTensorTonks
{
public:

  
  virtual ~ElasticityTensorR4() {}
  
  /**
  * fillFromInputVector takes either 21 (all=true) or 9 (all=false) inputs to fill in
  * the Rank-4 tensor with the appropriate crystal symmetries maintained. I.e., C_ijkl = C_klij,
  * C_ijkl = C_ijlk, C_ijkl = C_jikl
  */
  void fillFromInputVector(const std::vector<Real> input, bool all);
  
  virtual Real stiffness( const unsigned int i, const unsigned int j,
                          const RealGradient & test,
                          const RealGradient & phi);
  
  
protected:

private:

};

#endif //ELASTICITYTENSORR4_H
