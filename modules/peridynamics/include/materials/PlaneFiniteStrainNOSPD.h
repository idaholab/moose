//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PLANEFINITESTRAINNOSPD_H
#define PLANEFINITESTRAINNOSPD_H

#include "FiniteStrainNOSPD.h"

class PlaneFiniteStrainNOSPD;

template <>
InputParameters validParams<PlaneFiniteStrainNOSPD>();

/**
 * Material class for 2D correspondence material model for finite strain: plane strain, generalized
 * plane strain, weak plane stress
 */
class PlaneFiniteStrainNOSPD : public FiniteStrainNOSPD
{
public:
  PlaneFiniteStrainNOSPD(const InputParameters & parameters);

protected:
  virtual void computeQpFhat() override;

  ///@{Functions to compute the out-of-plane component of deformation gradient for generalized plane strain and weak plane stress
  virtual Real computeQpOutOfPlaneDeformationGradient();
  virtual Real computeQpOutOfPlaneDeformationGradientOld();
  ///@}
private:
  ///@{ Scalar out-of-plane strain for generalized plane strain
  const bool _scalar_out_of_plane_strain_coupled;
  const VariableValue & _scalar_out_of_plane_strain;
  const VariableValue & _scalar_out_of_plane_strain_old;
  ///@}

  ///@{ Out-of-plane strain for weak plane stress
  const bool _out_of_plane_strain_coupled;
  const VariableValue & _out_of_plane_strain;
  const VariableValue & _out_of_plane_strain_old;
  ///@}
};

#endif // PLANEFINITESTRAINNOSPD_H
