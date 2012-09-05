/**
 * RankFourTensorTonks is designed to handle any fourth order tensor.
 * It is designed to allow for maximum clarity of the mathematics and ease of use.
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 *
 * RankFourTensorTonks holds the 81 separate C_ijkl entries; the entries are accessed by index, with
 * i, j, k, and l equal to 0, 1, 2
 * 
 */

#ifndef RANKFOURTENSORTONKS_H
#define RANKFOURTENSORTONKS_H

// Any requisite includes here
#include "tensor_value.h"
#include <vector>
#include "libmesh.h"
#include "vector_value.h"

class RankFourTensorTonks
{
public:

  /**
   * Default constructor; fills to zero
   */
  RankFourTensorTonks();

  /**
   * Copy constructor
   */
  RankFourTensorTonks(const RankFourTensorTonks &a);

  ~RankFourTensorTonks() {}
  
  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   */
  Real & operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l);


  /**
   * Gets the value for the index specified.  Takes index = 0,1,2,
   * used for const
   */
  Real operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const;

  /**
  * Zeros out the tensor.
  */
  void zero();

  RankFourTensorTonks & operator=(const RankFourTensorTonks &a);
  
  RealTensorValue operator*(const RealTensorValue &a);

  RankFourTensorTonks operator*(const Real &a);

  RankFourTensorTonks & operator+=(const RankFourTensorTonks &a);
  
  RankFourTensorTonks operator+(const RankFourTensorTonks &a) const;
  
  RankFourTensorTonks & operator-=(const RankFourTensorTonks &a);
  
  RankFourTensorTonks operator-(const RankFourTensorTonks &a) const;

  virtual void rotate(RealTensorValue &R);
  
  
protected:

/**
 * Contains the actual data for the Rank Four tensor. 
 */
  static const unsigned int N = 3;
  
  Real _vals[N][N][N][N];

private:

};

#endif //RANKFOURTENSORTONKS_H
