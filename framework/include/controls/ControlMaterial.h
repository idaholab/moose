/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONTROLMATERIAL_H
#define CONTROLMATERIAL_H

// MOOSE includes
#include "Material.h"

// Forward declarations
class ControlMaterial;

template<>
InputParameters validParams<ControlMaterial>();

/**
 *
 */
class ControlMaterial : public Material
{
public:

  /**
   * Class constructor
   * @param parameters
   */
  ControlMaterial(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~ControlMaterial();

protected:


  template<typename T>
  MaterialProperty<T> & getControlMaterialProperty(const std::string & name);


};

template<typename T>
MaterialProperty<T> &
ControlMaterial::getControlMaterialProperty(const std::string & name)
{
  // This needs the logic for parsing object names...

  // This needs logic for handling defaults (use deducePropertyName)
  std::string prop_name = getParam<MaterialPropertyName>(name);


  // ERROR if property doesn't exist


  return _material_data.getProperty<T>(prop_name);
}


#endif // CONTROLMATERIAL_H
