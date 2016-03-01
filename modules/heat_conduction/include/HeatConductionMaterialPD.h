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

class HeatConductionMaterialPD : public Material
{
public:
  HeatConductionMaterialPD(const InputParameters & parameters);
  virtual ~HeatConductionMaterialPD();

protected:

  virtual void computeProperties();

  MooseVariable * _temp_var;
  
  const Real _my_thermal_conductivity;

  MaterialProperty<Real> & _thermal_conductivity;
  Function * _thermal_conductivity_function;

  const int _pddim;
  const Real _mesh_spacing;

  MaterialProperty<Real> & _bond_response;
  MaterialProperty<Real> & _bond_response_dif_temp;
  MaterialProperty<Real> & _node_volume;

};

#endif //HEATCONDUCTIONMATERIALPD_H
