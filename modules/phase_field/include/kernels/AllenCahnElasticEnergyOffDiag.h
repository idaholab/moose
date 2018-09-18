//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ALLENCAHNELASTICENERGYOFFDIAG_H
#define ALLENCAHNELASTICENERGYOFFDIAG_H

#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "Kernel.h"

// Forward Declarations
class AllenCahnElasticEnergyOffDiag;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

template <>
InputParameters validParams<AllenCahnElasticEnergyOffDiag>();

/**
 * This kernel computes the off-diagonal jacobian of elastic energy in AllenCahn respect to
 * displacements
 */
class AllenCahnElasticEnergyOffDiag
  : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  AllenCahnElasticEnergyOffDiag(const InputParameters & parameters);

protected:
  Real computeQpResidual() override { return 0.0; }

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Mobility
  const MaterialProperty<Real> & _L;

  ///@{ Displacement variables used for off-diagonal Jacobian
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  ///@}

  /// Free energy material properties and derivatives
  const MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
};
#endif // ALLENCAHNELASTICENERGYOFFDIAG_H
