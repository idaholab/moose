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

#include "MaterialFactory.h"
#include "MooseSystem.h"

//libMesh Includes
#include "equation_systems.h"

#include <iostream>

MaterialFactory::MaterialFactory()
{
}

MaterialFactory *
MaterialFactory::instance()
     {
    static MaterialFactory * instance;
    if(!instance)
      instance=new MaterialFactory;
    return instance;
  }

InputParameters
MaterialFactory::getValidParams(std::string name)
  {
    if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
      mooseError("A _" + name + "_ is not registered Material\n\n");
    return _name_to_params_pointer[name]();
  }

MaterialNamesIterator
MaterialFactory::registeredMaterialsBegin()
{
  // Make sure the _registered_kernel_names are up to date
  _registered_material_names.clear();
  _registered_material_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, MaterialParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_material_names.push_back(i->first);
  }
  
  return _registered_material_names.begin();
}

MaterialNamesIterator
MaterialFactory::registeredMaterialsEnd()
{
  return _registered_material_names.end();
}
