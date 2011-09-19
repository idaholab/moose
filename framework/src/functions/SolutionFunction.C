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

#include "Moose.h" //for mooseError
#include "SolutionFunction.h"

// libmesh includes
#include "MooseMesh.h"
#include "equation_systems.h"
#include "mesh_function.h"
#include "numeric_vector.h"

template<>
InputParameters validParams<SolutionFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("mesh", "The name of the mesh file in xda format");
  params.addRequiredParam<std::string>("es", "The name of the file holding the equation system info in xda format");
  params.addParam<std::string>("system", "NonlinearSystem", "The name of the system you want to pull values out of");
  params.addRequiredParam<std::string>("variable", "The name of the variable you want to use for values");
  return params;
}

SolutionFunction::SolutionFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _mesh_file(getParam<std::string>("mesh")),
    _es_file(getParam<std::string>("es")),
    _system_name(getParam<std::string>("system")),
    _var_name(getParam<std::string>("variable"))
{
  _mesh = new Mesh;
  _mesh->read(_mesh_file);

  _es = new EquationSystems(*_mesh);
  _es->read(_es_file);
  _es->update();

  _system = &_es->get_system(_system_name);
  std::cerr << "sys = " << _system << std::endl;

  _serialized_solution = NumericVector<Number>::build().release();
  _serialized_solution->init(_system->n_dofs(), false, SERIAL);

  // Need to pull down a full copy of this vector on every processor so we can get values in parallel
  _system->solution->localize(*_serialized_solution);

  _mesh_function = new MeshFunction(*_es, *_serialized_solution, _system->get_dof_map(), _system->variable_number(_var_name));

  _mesh_function->init();
}

SolutionFunction::~SolutionFunction()
{
  delete _es;
  delete _mesh;
  delete _serialized_solution;
  delete _mesh_function;
}

Real
SolutionFunction::value(Real, const Point & p)
{
  return (*_mesh_function)(p);
}
