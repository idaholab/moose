/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRACKPROPAGATIONHEATENERGY_H
#define CRACKPROPAGATIONHEATENERGY_H

#include "Kernel.h"
#include "RankTwoTensor.h"

// Forward Declarations
class CrackPropagationHeatEnergy;

template <>
InputParameters validParams<CrackPropagationHeatEnergy>();

/**
 * Provides a heat source from crack propagation:
 * - (dPsi/dc) * (dc/dt)
 * Psi is the free energy of the phase-field fracture model
 * defined as Psi = (1 - c)^2 * G0_pos + G0_neg
 * c is the order parameter for damage, continuous between 0 and 1
 * 0 represents no damage, 1 represents fully cracked
 * G0_pos and G0_neg are the positive and negative components
 * of the specific strain energies
 * - (dPsi/dc) * (dc/dt) = 2 * (1 - c) * G0_pos * (dc/dt)
 * C. Miehe, L.M. Schanzel, H. Ulmer, Comput. Methods Appl. Mech. Engrg. 294 (2015) 449 - 485
 * P. Chakraborty, Y. Zhang, M.R. Tonks, Multi-scale modeling of microstructure dependent
 * inter-granular fracture in UO2 using a phase-field based method
 * Idaho National Laboratory technical report
 */
class CrackPropagationHeatEnergy : public Kernel
{
public:
  CrackPropagationHeatEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// optional parameter that allows multiple mechanics models to be defined
  std::string _base_name;

  /// - (dPsi/dc) * (dc/dt) = 2 * (1 - c) * G0_pos * (dc/dt)
  const MaterialProperty<Real> & _crack_propagation_heat;

  /// d(crack_propagation_heat)/d(total strain)
  const MaterialProperty<RankTwoTensor> & _dcrack_propagation_heat_dstrain;

  /// d(crack_propagation_heat)/d(c)
  const MaterialProperty<Real> & _dcrack_propagation_heat_dc;

  /// MOOSE variable number for the phase field damage variable
  const unsigned int _c_var;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// MOOSE variable number for the displacement variables
  std::vector<unsigned int> _disp_var;
};

#endif // CRACKPROPAGATIONHEATENERGY_H
