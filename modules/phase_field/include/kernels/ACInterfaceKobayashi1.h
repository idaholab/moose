/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACEKOBAYASHI1_H
#define ACINTERFACEKOBAYASHI1_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterfaceKobayashi1;

template<>
InputParameters validParams<ACInterfaceKobayashi1>();

class ACInterfaceKobayashi1 : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
  ACInterfaceKobayashi1(const InputParameters & parameters);

protected:

  /// Enum of computeDFDOP inputs
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;
  const MaterialProperty<Real> & _eps;
  const MaterialProperty<Real> & _deps;
  const MaterialProperty<RealGradient> & _depsdgrad_op;
  const MaterialProperty<RealGradient> & _ddepsdgrad_op;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
  std::vector<const MaterialProperty<Real> *> _ddepsdarg;
};

#endif //ACINTERFACEKOBAYASHI1_H
