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

#include "StabilizerFactory.h"

StabilizerFactory *
StabilizerFactory::instance()
{
  static StabilizerFactory * instance;
  if(!instance)
    instance=new StabilizerFactory;
    
  return instance;
}

InputParameters
StabilizerFactory::getValidParams(std::string name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    std::cerr<<std::endl<<"A _"<<name<<"_ is not a registered Stabilizer "<<std::endl<<std::endl;
    mooseError("");
  }
  return _name_to_params_pointer[name]();
}

StabilizerFactory::StabilizerFactory()
{
}
  
StabilizerFactory:: ~StabilizerFactory() 
{
  {
    std::map<std::string, stabilizerBuildPtr>:: iterator i;
    for(i=_name_to_build_pointer.begin(); i!=_name_to_build_pointer.end(); ++i)
    {
      delete &i;
    }
  }

  {
    std::map<std::string, stabilizerParamsPtr>::iterator i;
    for(i=_name_to_params_pointer.begin(); i!=_name_to_params_pointer.end(); ++i)
    {
      delete &i;
    }
  }
}

StabilizerNamesIterator
StabilizerFactory::registeredStabilizersBegin()
{
  // Make sure the _registered_stabilizer_names are up to date
  _registered_stabilizer_names.clear();
  _registered_stabilizer_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, stabilizerParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_stabilizer_names.push_back(i->first);
  }

  return _registered_stabilizer_names.begin();
}

StabilizerNamesIterator
StabilizerFactory::registeredStabilizersEnd()
{
  return _registered_stabilizer_names.end();
}
