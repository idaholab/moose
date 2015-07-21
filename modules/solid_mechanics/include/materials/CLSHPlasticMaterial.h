/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CLSHPLASTICMATERIAL_H
#define CLSHPLASTICMATERIAL_H

#include "SolidModel.h"

/**
 * Plastic material
 */
class CLSHPlasticMaterial : public SolidModel
{
public:
  CLSHPlasticMaterial(const InputParameters & parameters);

  CLSHPlasticMaterial(std::string name,
                      InputParameters parameters); // DEPRECATED

protected:

};

template<>
InputParameters validParams<CLSHPlasticMaterial>();

#endif //CLSHPLASTICMATERIAL_H
