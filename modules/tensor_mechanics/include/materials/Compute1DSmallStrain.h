/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTE1DSMALLSTRAIN_H
#define COMPUTE1DSMALLSTRAIN_H

#include "ComputeSmallStrain.h"

/**
 * Compute1DSmallStrain defines a strain tensor, assuming small strains,
 * in 1D problems, handling strains in other two directions.
 * Compute1DSmallStrain contains virtual methods to define the strain_yy and strain_zz
 * as a general nonzero value.
 */
class Compute1DSmallStrain : public ComputeSmallStrain
{
public:
  Compute1DSmallStrain(const InputParameters & parameters);

protected:
  void computeProperties() override;

  /// Computes the strain_yy; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeStrainYY() = 0;

  /// Computes the strain_zz; as a virtual function, this function is
  /// overwritten for the specific geometries defined by inheriting classes
  virtual Real computeStrainZZ() = 0;
};

#endif // COMPUTE1DSMALLSTRAIN_H
