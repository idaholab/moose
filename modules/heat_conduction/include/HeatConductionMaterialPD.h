/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATCONDUCTIONMATERIALPD_H
#define HEATCONDUCTIONMATERIALPD_H

#include "Material.h"

//Forward Declarations
class HeatConductionMaterialPD;
class Function;

template<>
InputParameters validParams<HeatConductionMaterialPD>();

/**
 * Simple material with constant properties.
 */
class HeatConductionMaterialPD : public Material
{
public:
  HeatConductionMaterialPD(const InputParameters & parameters);
  virtual ~HeatConductionMaterialPD();

protected:
  MooseVariable * _temp_var;
  
  virtual void computeProperties();

  const Real _my_thermal_conductivity;
  const Real _my_specific_heat;

  MaterialProperty<Real> & _thermal_conductivity;
  Function * _thermal_conductivity_function;

  MaterialProperty<Real> & _specific_heat;
  Function * _specific_heat_function;

  MaterialProperty<Real> & _mass_density;
  
  const int _pddim;
  const Real _mesh_spacing;
  const Real _domain_thickness;

  MaterialProperty<Real> & _bond_response;
  MaterialProperty<Real> & _bond_response_dif;
  MaterialProperty<Real> & _bond_volume;

};

#endif //HEATCONDUCTIONMATERIALPD_H
