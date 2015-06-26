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

//Forward Declarations
class ACBulk;

template<>
InputParameters validParams<ACBulk>();

class ACBulk : public DerivativeMaterialInterface<JvarMapInterface<KernelValue> >
{
public:
  ACBulk(const std::string & name, InputParameters parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };

  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeDFDOP(PFFunctionType type) = 0;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialPropertyName _mob_name;
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;

  std::vector<const MaterialProperty<Real> *> _dLdarg;
};

#endif //ACBULK_H
