/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVLEASTSQUARESSLOPERECONSTRUCTION_H
#define CNSFVLEASTSQUARESSLOPERECONSTRUCTION_H

#include "SlopeReconstructionMultiD.h"
#include "SinglePhaseFluidProperties.h"

// Forward Declarations
class CNSFVLeastSquaresSlopeReconstruction;

template <>
InputParameters validParams<CNSFVLeastSquaresSlopeReconstruction>();

/**
 * A user object that performs the least-squares slope reconstruction to get the slopes of the P0
 * primitive variables
 */
class CNSFVLeastSquaresSlopeReconstruction : public SlopeReconstructionMultiD
{
public:
  CNSFVLeastSquaresSlopeReconstruction(const InputParameters & parameters);

  /// compute the slope of the cell
  virtual void reconstructElementSlope();

protected:
  /// the input density
  MooseVariable * _rho;
  /// the input x-momentum
  MooseVariable * _rhou;
  /// the input y-momentum
  MooseVariable * _rhov;
  /// the input z-momentum
  MooseVariable * _rhow;
  /// the input total energy
  MooseVariable * _rhoe;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};

#endif
