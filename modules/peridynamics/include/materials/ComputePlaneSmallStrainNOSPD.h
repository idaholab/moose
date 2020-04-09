//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeSmallStrainNOSPD.h"
#include "RankTwoTensor.h"

/**
 * Material class for 2D correspondence material model for small strain: plane strain, generalized
 * plane strain, weak plane stress
 */
class ComputePlaneSmallStrainNOSPD : public ComputeSmallStrainNOSPD
{
public:
  static InputParameters validParams();

  ComputePlaneSmallStrainNOSPD(const InputParameters & parameters);

protected:
  virtual void computeQpTotalStrain() override;

  /**
   * Function to compute out-of-plane component of strain tensor for generalized plane strain and
   * weak plane stress
   * @return The value of out-of-plane strain
   */
  Real computeOutOfPlaneStrain();

private:
  ///@{ Scalar out-of-plane strain for generalized plane strain
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  ///@}

  ///@{ Out-of-plane strain for weak plane stress
  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  ///@}
};
