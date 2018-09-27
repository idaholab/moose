//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MAXPRINCIPALSTRESSNOSPD_H
#define MAXPRINCIPALSTRESSNOSPD_H

#include "AuxKernelBasePD.h"
#include "RankTwoTensor.h"

class MaxPrincipalStressNOSPD;

template <>
InputParameters validParams<MaxPrincipalStressNOSPD>();

/**
 * Aux Kernel class to output the bond maximum principal stress value
 */
class MaxPrincipalStressNOSPD : public AuxKernelBasePD
{
public:
  MaxPrincipalStressNOSPD(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// bond_status variable
  const MooseVariableFEBase & _bond_status_var;

  /// Stress material property
  const MaterialProperty<RankTwoTensor> & _stress;
};

#endif // MAXPRINCIPALSTRESSNOSPD_H
