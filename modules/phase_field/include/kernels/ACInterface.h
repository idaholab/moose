/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACInterface_H
#define ACInterface_H

#include "KernelGrad.h"

//Forward Declarations
class ACInterface;

template<>
InputParameters validParams<ACInterface>();

class ACInterface : public KernelGrad
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
  std::string _mob_name;
  std::string _kappa_name;


private:
  const MaterialProperty<Real> & _kappa;
  const MaterialProperty<Real> & _L;
};

#endif //ACInterface_H
