/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVWENOSLOPELIMITING_H
#define CNSFVWENOSLOPELIMITING_H

#include "SlopeLimitingBase.h"

// Forward Declarations
class CNSFVWENOSlopeLimiting;

template <>
InputParameters validParams<CNSFVWENOSlopeLimiting>();

/**
 * A user object that performs WENO slope limiting to get the limited slopes of cell average
 * variables in multi-dimensions
 *
 * Reference article
 *
 * Xia, Y., Liu, X., & Luo, H. (2014).
 * A finite volume method based on a WENO reconstruction
 * for compressible flows on hybrid grids.
 * In 52nd AIAA Aerospace Sciences Meeting,
 * AIAA Paper No. 2014-0939
 */
class CNSFVWENOSlopeLimiting : public SlopeLimitingBase
{
public:
  CNSFVWENOSlopeLimiting(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const;

protected:
  /// linear weight
  Real _lweig;
  /// decay power
  Real _dpowe;
};

#endif
