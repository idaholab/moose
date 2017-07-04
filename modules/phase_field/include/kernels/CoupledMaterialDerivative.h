/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDMATERIALDERIVATIVE_H
#define COUPLEDMATERIALDERIVATIVE_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declaration
class CoupledMaterialDerivative;

template <>
InputParameters validParams<CoupledMaterialDerivative>();

/**
 * This kernel adds the term (dFdv, test), where v is a coupled variable.
 */
class CoupledMaterialDerivative : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  CoupledMaterialDerivative(const InputParameters & parameters);
  virtual void initialSetup() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  std::string _v_name;
  unsigned int _v_var;

  /// Free Energy derivative w.r.t. v
  const MaterialProperty<Real> & _dFdv;

  /// Free energy 2nd order derivative w.r.t. v then u
  const MaterialProperty<Real> & _d2Fdvdu;

  /// Free energy 2nd order derivative w.r.t. v
  const MaterialProperty<Real> & _d2Fdv2;

  /// Number of coupled variables
  const unsigned int _nvar;

  /// Reaction rate derivatives w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _d2Fdvdarg;
};

#endif // COUPLEDMATERIALDERIVATIVE_H
