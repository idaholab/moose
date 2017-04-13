/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACINTERFACEKOBAYASHI2_H
#define ACINTERFACEKOBAYASHI2_H

#include "KernelGrad.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

class ACInterfaceKobayashi2;

template <>
InputParameters validParams<ACInterfaceKobayashi2>();

/**
 * Kernel 2 of 2 for interfacial energy anisotropy in the Allen-Cahn equation as
 * implemented in R. Kobayashi, Physica D, 63, 410-423 (1993).
 * doi:10.1016/0167-2789(93)90120-P
 * This kernel implements the third term on the right side of eq. (3) of the paper.
 */
class ACInterfaceKobayashi2 : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelGrad>>
{
public:
  ACInterfaceKobayashi2(const InputParameters & parameters);

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  const MaterialProperty<Real> & _dLdop;

  /// Interfacial parameter
  const MaterialProperty<Real> & _eps;
  const MaterialProperty<RealGradient> & _depsdgrad_op;

  /// Mobility derivative w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dLdarg;
  std::vector<const MaterialProperty<Real> *> _depsdarg;
};

#endif // ACINTERFACEKOBAYASHI2_H
