/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACEKOBAYASHI1_H
#define ACINTERFACEKOBAYASHI1_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterfaceKobayashi1;

template <>
InputParameters validParams<ACInterfaceKobayashi1>();

/**
 * Kernel 1 of 2 for interfacial energy anisotropy in the Allen-Cahn equation as
 * implemented in R. Kobayashi, Physica D, 63, 410-423 (1993).
 * doi:10.1016/0167-2789(93)90120-P
 * This kernel implements the first two terms on the right side of eq. (3) of the paper.
 */
class ACInterfaceKobayashi1 : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>
{
public:
  ACInterfaceKobayashi1(const InputParameters & parameters);

protected:
  /// Enum of computeDFDOP inputs
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;
  const MaterialProperty<Real> & _eps;
  const MaterialProperty<Real> & _deps;
  const MaterialProperty<RealGradient> & _depsdgrad_op;
  const MaterialProperty<RealGradient> & _ddepsdgrad_op;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
  std::vector<const MaterialProperty<Real> *> _ddepsdarg;
};

#endif // ACINTERFACEKOBAYASHI1_H
