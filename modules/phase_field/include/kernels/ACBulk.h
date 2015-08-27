/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACBULK_H
#define ACBULK_H

#include "KernelValue.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACBulk;

template<>
InputParameters validParams<ACBulk>();

class ACBulk : public DerivativeMaterialInterface<JvarMapInterface<KernelValue> >
{
public:
  ACBulk(const InputParameters & parameters);

  virtual void initialSetup();

protected:

  /// Enum used with computeDFDOP function
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeDFDOP(PFFunctionType type) = 0;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;

  /// Mobility derivatives w.r.t. order parameter
  const MaterialProperty<Real> & _dLdop;

  /// Mobility derivative w.r.t coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
};

#endif //ACBULK_H
