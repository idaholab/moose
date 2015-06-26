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

//Forward Declarations
class ACInterface;

template<>
InputParameters validParams<ACInterface>();

class ACInterface : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
  ACInterface(const std::string & name, InputParameters parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _kappa;
  const MaterialPropertyName _mob_name;
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;

  std::vector<const MaterialProperty<Real> *> _dLdarg;
};

#endif //ACInterface_H
