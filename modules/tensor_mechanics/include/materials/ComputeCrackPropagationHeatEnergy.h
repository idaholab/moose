/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTECRACKPROPAGATIONHEATENERGY_H
#define COMPUTECRACKPROPAGATIONHEATENERGY_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankTwoTensor.h"

/**
 * ComputeCrackPropagationHeatEnergy computes the energy dissipated due to damage increase = -
 * (dPsi/dc) * (dc/dt)
 * and, if currentlyComputingJacobian, then the derivative of this quantity wrt total strain
 * Psi is the free energy of the phase-field fracture model
 * defined as Psi = (1 - c)^2 * G0_pos + G0_neg
 * c is the order parameter for damage, continuous between 0 and 1
 * 0 represents no damage, 1 represents fully cracked
 * G0_pos and G0_neg are the positive and negative components
 * of the specific strain energies
 * C. Miehe, L.M. Schanzel, H. Ulmer, Comput. Methods Appl. Mech. Engrg. 294 (2015) 449 - 485
 * P. Chakraborty, Y. Zhang, M.R. Tonks, Multi-scale modeling of microstructure dependent
 * inter-granular fracture in UO2 using a phase-field based method
 * Idaho National Laboratory technical report
 */
class ComputeCrackPropagationHeatEnergy : public DerivativeMaterialInterface<Material>
{
public:
  ComputeCrackPropagationHeatEnergy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// optional parameter that allows multiple mechanics materials to be defined
  std::string _base_name;

  /// Phase field damage variable
  const VariableValue & _c;

  /// old value of phase field damage variable
  const VariableValue & _c_old;

  /// Contribution of umdamaged strain energy to damage evolution
  const MaterialProperty<Real> & _G0_pos;

  /// Variation of undamaged strain energy driving damage evolution with strain
  const MaterialProperty<RankTwoTensor> & _dG0_pos_dstrain;

  /// computed property: 2 * (1 - c) * G0_pos * (dc/dt)
  MaterialProperty<Real> & _crack_propagation_heat;

  /// d(crack_propagation_heat)/d(total strain)
  MaterialProperty<RankTwoTensor> & _dcrack_propagation_heat_dstrain;

  /// d(crack_propagation_heat)/d(c)
  MaterialProperty<Real> & _dcrack_propagation_heat_dc;
};

#endif // COMPUTECRACKPROPAGATIONHEATENERGY_H
