/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PLASTICHEATENERGY_H
#define PLASTICHEATENERGY_H

#include "Kernel.h"
#include "RankTwoTensor.h"

// Forward Declarations
class PlasticHeatEnergy;

template <>
InputParameters validParams<PlasticHeatEnergy>();

/**
 * Provides a heat source from plastic deformation:
 * coeff * stress * plastic_strain_rate
 */
class PlasticHeatEnergy : public Kernel
{
public:
  PlasticHeatEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// coefficient of stress * plastic_strain_rate
  Real _coeff;

  /// optional parameter that allows multiple mechanics models to be defined
  std::string _base_name;

  /// stress * plastic_strain_rate
  const MaterialProperty<Real> & _plastic_heat;

  /// d(plastic_heat)/d(total_strain)
  const MaterialProperty<RankTwoTensor> & _dplastic_heat_dstrain;

  /// umber of coupled displacement variables
  unsigned int _ndisp;

  /// MOOSE variable number for the displacement variables
  std::vector<unsigned int> _disp_var;
};

#endif // PLASTICHEATENERGY_H
