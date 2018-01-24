/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ALLENCAHNPFFRACTURE_H
#define ALLENCAHNPFFRACTURE_H

#include "DerivativeMaterialInterface.h"
#include "JvarMapInterface.h"
#include "Kernel.h"

// Forward Declarations
class AllenCahnPFFracture;
class RankTwoTensor;

template <>
InputParameters validParams<AllenCahnPFFracture>();

/**
 * Phase field based fracture model
 * This kernel computes the residual and jacobian for bulk free energy contribution to c
 * Refer to Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311 Equation 63
 */
class AllenCahnPFFracture : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  AllenCahnPFFracture(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// Critical energy release rate for fracture
  const MaterialProperty<Real> & _gc_prop;

  ///@{ Coupled variable that is the laplacian of c
  const VariableValue & _beta;
  const unsigned int _beta_var;
  ///@}

  ///@{ Displacement variables used for off-diagonal Jacobian
  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  ///@}

  /// Characteristic length, controls damage zone thickness
  const MaterialProperty<Real> & _l;

  /// Viscosity parameter ( visco -> 0, rate independent )
  const MaterialProperty<Real> & _visco;

  ///@{ Free energy material properties and derivatives
  const MaterialProperty<Real> & _dFdc;
  const MaterialProperty<Real> & _d2Fdc2;
  const MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;
  //@}
};
#endif // ALLENCAHNPFFRACTURE_H
