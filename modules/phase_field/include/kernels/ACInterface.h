/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACInterface_H
#define ACInterface_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterface;

template<>
InputParameters validParams<ACInterface>();

class ACInterface : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
  ACInterface(const std::string & name, InputParameters parameters);

protected:

  /// Enum of computeDFDOP inputs
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;

  /// Name of mobility material property
  const MaterialPropertyName _mob_name;

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// Mobility derivative w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
};

#endif //ACInterface_H
