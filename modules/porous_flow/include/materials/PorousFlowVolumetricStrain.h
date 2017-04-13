/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWVOLUMETRICSTRAIN_H
#define POROUSFLOWVOLUMETRICSTRAIN_H

#include "PorousFlowMaterialVectorBase.h"
#include "RankTwoTensor.h"

/**
 * PorousFlowVolumetricStrain computes volumetric strains, and derivatives thereof
 */
class PorousFlowVolumetricStrain : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowVolumetricStrain(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// If true then the strain rate will include terms that ensure mass is conserved when doing integrals over the displaced mesh
  const bool _consistent;

  /// number of displacements supplied (1 in 1D, 2 in 2D, 3 in 3D)
  const unsigned int _ndisp;

  /// displacement variable values at the quad point
  std::vector<const VariableValue *> _disp;

  /// moose variable number of the displacements variables provided
  std::vector<unsigned int> _disp_var_num;

  /// gradient of the displacements
  std::vector<const VariableGradient *> _grad_disp;

  /// old value of gradient of the displacements
  std::vector<const VariableGradient *> _grad_disp_old;

  /// The volumetric strain rate at the quadpoints
  MaterialProperty<Real> & _vol_strain_rate_qp;

  /**
   * The derivative of the volumetric strain rate with respect to the porous flow variables.
   * Since the volumetric strain rate depends on derivatives of the displacement variables,
   * this should be multiplied by _grad_phi in kernels
   */
  MaterialProperty<std::vector<RealGradient>> & _dvol_strain_rate_qp_dvar;

  /// The total volumetric strain at the quadpoints
  MaterialProperty<Real> & _vol_total_strain_qp;

  /**
   * The derivative of the total volumetric strain with respect to the porous flow variables.
   * Since the total volumetric strain depends on derivatives of the displacement variables,
   * this should be multiplied by _grad_phi in kernels
   */
  MaterialProperty<std::vector<RealGradient>> & _dvol_total_strain_qp_dvar;
};

#endif // POROUSFLOWVOLUMETRICSTRAIN_H
