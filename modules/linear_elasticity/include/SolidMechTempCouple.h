/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLIDMECHTEMPCOUPLE_H
#define SOLIDMECHTEMPCOUPLE_H

#include "SolidMech.h"

//libMesh includes
#include "libmesh/tensor_value.h"

//Forward Declarations
class SolidMechTempCouple;

template<>
InputParameters validParams<SolidMechTempCouple>();

class SolidMechTempCouple : public SolidMech
{
public:

  SolidMechTempCouple(const InputParameters & parameters);

  virtual void subdomainSetup();

  void recomputeCouplingConstants();

protected:
  unsigned int _temp_var;

  const MaterialProperty<Real> & _thermal_strain;
  const MaterialProperty<Real> & _alpha;

  Real _c4;

  const unsigned int _mesh_dimension;
};


#endif //SOLIDMECHTEMPCOUPLE_H
