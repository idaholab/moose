/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef anisoACInterface2_H
#define anisoACInterface2_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class anisoACInterface2;

template<>
InputParameters validParams<anisoACInterface2>();

class anisoACInterface2 : public DerivativeMaterialInterface<JvarMapInterface<KernelGrad> >
{
public:
 anisoACInterface2(const std::string & name, InputParameters parameters);

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
      
  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
 };

#endif //anisoACInterface2_H
