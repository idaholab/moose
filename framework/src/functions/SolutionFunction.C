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
#include "nonlinear_implicit_system.h"
#include "transient_system.h"

template<>
InputParameters validParams<SolutionFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("mesh", "The name of the mesh file.");
  params.addParam<std::string>("file_type","xda","The type of format that is to be read (xda | exodusII).");
  params.addParam<std::string>("es", "The name of the file holding the equation system info in xda format.");
  params.addParam<std::string>("system", "NonlinearSystem", "The name of the system you want to pull values out of.");
  params.addRequiredParam<std::string>("variable", "The name of the variable you want to use for values.");
  params.addParam<int>("timestep", 1, "The timestep to read (only necessary for ExodusII)");
  return params;
}

SolutionFunction::SolutionFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _mesh_file(getParam<std::string>("mesh")),
    _file_type(getParam<std::string>("file_type")),
    _es_file(getParam<std::string>("es")),
    _system_name(getParam<std::string>("system")),
    _var_name(getParam<std::string>("variable")),
    _local_timestep(getParam<int>("timestep")),
    _mesh(NULL),
    _es(NULL),
    _system(NULL),
    _mesh_function(NULL),
    _exodusII_io(NULL),
    _serialized_solution(NULL)
{
  // Set up file type list of file types we support
  _internal_file_type_list["xda"] = 1;
  _internal_file_type_list["exodusII"] = 2;

  _mesh = new Mesh;

  switch( _internal_file_type_list[_file_type])
  {
     case 1:
       _mesh->read(_mesh_file);
       _es = new EquationSystems(*_mesh);
       _es->read(_es_file);
       _es->update();
       _system = &_es->get_system(_system_name);
     break;
     case 2:
       _mesh = new Mesh;
       _exodusII_io = new ExodusII_IO (*_mesh);
       _exodusII_io->read(_mesh_file);
       _mesh->prepare_for_use();

       _es = new EquationSystems(*_mesh);
       _es->add_system<TransientExplicitSystem> (_system_name);
       _es->get_system(_system_name).add_variable(_var_name, FIRST);
       _es->init();
       _system = &_es->get_system(_system_name);

       _exodusII_io->copy_nodal_solution(*_system,_var_name,_local_timestep);

       _system->update();
       _es->update();
    break;

    default:
        mooseError("File type : "+_file_type+" not recognised");

    break;
  }

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
  delete _exodusII_io;
}

Real
SolutionFunction::value(Real, const Point & p)
{
  return (*_mesh_function)(p);
}
