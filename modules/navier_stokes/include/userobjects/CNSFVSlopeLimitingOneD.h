/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVSLOPELIMITINGONED_H
#define CNSFVSLOPELIMITINGONED_H

#include "SlopeLimitingBase.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVSlopeLimitingOneD;

template <>
InputParameters validParams<CNSFVSlopeLimitingOneD>();

/**
 * A use object that serves as base class for slope limiting to get the limited slopes of cell
 * average variables in 1-D
 */
class CNSFVSlopeLimitingOneD : public SlopeLimitingBase
{
public:
  CNSFVSlopeLimitingOneD(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const;

protected:
  /// the input density
  MooseVariable * _rho;
  /// the input x-momentum
  MooseVariable * _rhou;
  /// the input total energy
  MooseVariable * _rhoe;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// One-D slope limiting scheme
  MooseEnum _scheme;
};

#endif
