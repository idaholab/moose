/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
