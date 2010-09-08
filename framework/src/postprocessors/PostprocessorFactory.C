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

#include "PostprocessorFactory.h"
#include "MooseSystem.h"

PostprocessorFactory *
PostprocessorFactory::instance()
{
  static PostprocessorFactory * instance;
  if(!instance)
    instance=new PostprocessorFactory;
    
  return instance;
}

InputParameters
PostprocessorFactory::getValidParams(const std::string & name)
{
  if( _name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
  {
    mooseError(std::string("A _") + name + "_ is not a registered Postprocessor\n\n");
  }

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

PostprocessorNamesIterator
PostprocessorFactory::registeredPostprocessorsBegin()
{
  // Make sure the _registered_postprocessor_names are up to date
  _registered_postprocessor_names.clear();
  _registered_postprocessor_names.reserve(_name_to_params_pointer.size());

  // build a vector of strings from the params pointer map
  for (std::map<std::string, postprocessorParamsPtr>::iterator i = _name_to_params_pointer.begin();
       i != _name_to_params_pointer.end();
       ++i)
  {
    _registered_postprocessor_names.push_back(i->first);
  }

  return _registered_postprocessor_names.begin();
}

PostprocessorNamesIterator
PostprocessorFactory::registeredPostprocessorsEnd()
{
  return _registered_postprocessor_names.end();
}


PostprocessorFactory::PostprocessorFactory()
{
}
  
PostprocessorFactory:: ~PostprocessorFactory() 
{}

