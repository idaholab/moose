/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef anisoACInterface1_H
#define anisoACInterface1_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class anisoACInterface1;

template<>
InputParameters validParams<anisoACInterface1>();

class anisoACInterface1 : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
  anisoACInterface1(const std::string & name, InputParameters parameters);

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
  


  /// Mobility
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;
  const MaterialProperty<Real> & _eps;
  const MaterialProperty<Real> & _eps1;

        
  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
  std::vector<const MaterialProperty<Real> *> _deps1darg;
};

#endif //anisoACInterface1_H
