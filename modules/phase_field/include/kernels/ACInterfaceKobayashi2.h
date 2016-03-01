/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACEKOBAYASHI2_H
#define ACINTERFACEKOBAYASHI2_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterfaceKobayashi2;

template<>
InputParameters validParams<ACInterfaceKobayashi2>();

class ACInterfaceKobayashi2 : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
 ACInterfaceKobayashi2(const InputParameters & parameters);

protected:

  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;

  /// Interfacial parameter
  const MaterialProperty<Real> & _eps;
  const MaterialProperty<RealGradient> & _depsdgrad_op;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
 };

#endif //ACINTERFACEKOBAYASHI2_H
