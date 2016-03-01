/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEATSOURCEPD_H
#define HEATSOURCEPD_H

#include "Kernel.h"

//Forward Declaration
class HeatSourcePD;

template<>
InputParameters validParams<HeatSourcePD>();

class HeatSourcePD : public Kernel
{
public:

  HeatSourcePD(const InputParameters & parameters);
  virtual ~HeatSourcePD();

protected:

  MooseVariable * _total_bonds_var;

  virtual void computeResidual();

  virtual Real computeQpResidual() {return 0;}
 
  const MaterialProperty<Real> & _node_volume;
 
  Real _power_density;

  Function * _power_density_function;

};

#endif //HEATSOURCEPD_H
