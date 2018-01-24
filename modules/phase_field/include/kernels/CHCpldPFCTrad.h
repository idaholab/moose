//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHCPLDPFCTRAD_H
#define CHCPLDPFCTRAD_H

#include "LaplacianSplit.h"

// Forward Declarations
class CHCpldPFCTrad;

template <>
InputParameters validParams<CHCpldPFCTrad>();

class CHCpldPFCTrad : public LaplacianSplit
{
public:
  CHCpldPFCTrad(const InputParameters & parameters);

protected:
  virtual RealGradient precomputeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _coeff;
};

#endif // CHCPLDPFCTRAD_H
