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

#include "ProblemFactory.h"
#include "MooseMesh.h"
#include "FEProblem.h"

ProblemFactory * ProblemFactory::_instance = NULL;

ProblemFactory *ProblemFactory::instance()
{
  if (!_instance)
    _instance = new ProblemFactory;

  return _instance;
}

ProblemFactory::~ProblemFactory()
{
}

void
ProblemFactory::release()
{
  delete _instance;
}

InputParameters
ProblemFactory::getValidParams(const std::string & name)
{
  if (_name_to_params_pointer.find(name) == _name_to_params_pointer.end() )
    mooseError(std::string("A '") + name + "' is not a registered problem\n\n");

  InputParameters params = _name_to_params_pointer[name]();
  return params;
}

Problem *
ProblemFactory::create(const std::string & obj_name, const std::string & name, InputParameters parameters)
{
  if (_name_to_build_pointer.find(obj_name) == _name_to_build_pointer.end())
    mooseError("Problem '" + obj_name + "' was not registered.");

  return (*_name_to_build_pointer[obj_name])(name, parameters);
}

FEProblem *
ProblemFactory::createFEProblem(MooseMesh *mesh)
{
  InputParameters params = validParams<FEProblem>();
  params.set<std::string>("name") = "Moose Problem";
  params.set<MooseMesh *>("mesh") = mesh;
  return static_cast<FEProblem *>(create("FEProblem", "Moose Problem", params));
}



