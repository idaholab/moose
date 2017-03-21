/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALDERIVATIVETESTKERNEL_H
#define MATERIALDERIVATIVETESTKERNEL_H

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class MaterialDerivativeTestKernel;

template <>
InputParameters validParams<MaterialDerivativeTestKernel>();

/**
 * This kernel is used for testing derivatives of a material property.
 */
class MaterialDerivativeTestKernel
    : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  MaterialDerivativeTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// number of nonlinear variables
  const unsigned int _n_vars;
  /// material property for which to test derivatives
  const MaterialProperty<Real> & _p;
  /// material properties for the derivatives of the tested property
  std::vector<const MaterialProperty<Real> *> _p_derivatives;
};

#endif /* MATERIALDERIVATIVETESTKERNEL_H */
