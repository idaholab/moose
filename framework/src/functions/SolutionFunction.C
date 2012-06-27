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
  params.addParam<std::string>("es", "The name of the file holding the equation system info in xda format (xda only).");
  params.addParam<std::string>("system", "NonlinearSystem", "The name of the system to pull values out of (xda only).");
  params.addRequiredParam<std::string>("variable", "The name of the variable you want to use for values.");
  params.addParam<int>("timestep", -1, "Index of the single timestep used (exodusII only).  If not supplied, time interpolation will occur.");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the values");
  return params;
}

SolutionFunction::SolutionFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _mesh_file(getParam<std::string>("mesh")),
    _file_type(getSolutionFileType(getParam<std::string>("file_type"))),
    _es_file(getParam<std::string>("es")),
    _system_name(getParam<std::string>("system")),
    _var_name(getParam<std::string>("variable")),
    _exodus_time_index(getParam<int>("timestep")),
    _interpolate_times(false),
    _mesh(NULL),
    _es(NULL),
    _system(NULL),
    _mesh_function(NULL),
    _exodusII_io(NULL),
    _serialized_solution(NULL),
    _es2(NULL),
    _system2(NULL),
    _mesh_function2(NULL),
    _serialized_solution2(NULL),
    _interpolation_time(0.0),
    _interpolation_factor(0.0),
    _exodus_times(NULL),
    _exodus_index1(-1),
    _exodus_index2(-1),
    _scale_factor( getParam<Real>("scale_factor") )
{
  _mesh = new Mesh;

  if (_file_type == XDA)
  {
    if (_es_file == "")
      mooseError("In SolutionFunction, es must be supplied when file_type=xda");
    if (_system_name == "")
      mooseError("In SolutionFunction, system must be supplied when file_type=xda");
    _mesh->read(_mesh_file);
    _es = new EquationSystems(*_mesh);
    _es->read(_es_file);
    _es->update();
    _system = &_es->get_system(_system_name);
  }
  else if (_file_type == EXODUSII)
  {
    if (_system_name == "")
      _system_name = "SolutionFunctionSystem";
    if (_exodus_time_index == -1) //Interpolate between times rather than using values from a set timestep
      _interpolate_times = true;

    _exodusII_io = new ExodusII_IO (*_mesh);
    _exodusII_io->read(_mesh_file);
    _exodus_times = &_exodusII_io->get_time_steps();
    int num_exo_times = _exodus_times->size();
    if (num_exo_times == 0)
      mooseError("In SolutionFunction, exodus file contains no timesteps.");
    _mesh->prepare_for_use();

    _es = new EquationSystems(*_mesh);
    _es->add_system<ExplicitSystem> (_system_name);
    _es->get_system(_system_name).add_variable(_var_name, FIRST);
    _es->init();
    _system = &_es->get_system(_system_name);

    if (_interpolate_times)
    {
      _es2 = new EquationSystems(*_mesh);
      _es2->add_system<ExplicitSystem> (_system_name);
      _es2->get_system(_system_name).add_variable(_var_name, FIRST);
      _es2->init();
      _system2 = &_es2->get_system(_system_name);

      updateExodusBracketingTimeIndices(0.0);

      _exodusII_io->copy_nodal_solution(*_system,_var_name,_exodus_index1+1);
      _system->update();
      _es->update();

      _exodusII_io->copy_nodal_solution(*_system2,_var_name,_exodus_index2+1);
      _system2->update();
      _es2->update();

      _serialized_solution2 = NumericVector<Number>::build().release();
      _serialized_solution2->init(_system2->n_dofs(), false, SERIAL);
      // Need to pull down a full copy of this vector on every processor so we can get values in parallel
      _system2->solution->localize(*_serialized_solution2);
      _mesh_function2 = new MeshFunction(*_es2, *_serialized_solution2, _system2->get_dof_map(), _system2->variable_number(_var_name));
      _mesh_function2->init();
    }
    else
    {
      if (_exodus_time_index > num_exo_times)
        mooseError("In SolutionFunction, timestep = "<<_exodus_time_index<<", but there are only "<<num_exo_times<<" time steps.");
      _exodusII_io->copy_nodal_solution(*_system,_var_name,_exodus_time_index);
      _system->update();
      _es->update();
    }
  }
  else
  {
    mooseError("In SolutionFunction, invalid file type");
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
  if (_exodusII_io)
    delete _exodusII_io;
  if (_es2)
    delete _es2;
  if (_mesh_function2)
    delete _mesh_function2;
  if (_serialized_solution2)
    delete _serialized_solution2;
}

SolutionFunction::SolutionFileType
SolutionFunction::getSolutionFileType(const std::string filetype)
{
  if (filetype == "xda")
    return XDA;
  else if (filetype == "exodusII")
    return EXODUSII;
  else
    mooseError("In SolutionFunction, file type: "+filetype+" not recognized");
  return UNDEFINED;
}

void
SolutionFunction::updateExodusTimeInterpolation(Real time)
{
  if (time != _interpolation_time)
  {
    if (updateExodusBracketingTimeIndices(time))
    {
      _exodusII_io->copy_nodal_solution(*_system,_var_name,_exodus_index1+1);
      _system->update();
      _es->update();
      _system->solution->localize(*_serialized_solution);

      _exodusII_io->copy_nodal_solution(*_system2,_var_name,_exodus_index2+1);
      _system2->update();
      _es2->update();
      _system2->solution->localize(*_serialized_solution2);
    }
    _interpolation_time = time;
  }
}

bool
SolutionFunction::updateExodusBracketingTimeIndices(Real time)
{
  if (_file_type != EXODUSII)
    mooseError("In SolutionFunction, getTimeInterpolationData only applicable for exodusII file type");

  int old_index1 = _exodus_index1;
  int old_index2 = _exodus_index2;

  int num_exo_times = _exodus_times->size();

  if (time  < (*_exodus_times)[0])
  {
    _exodus_index1 = 0;
    _exodus_index2 = 0;
    _interpolation_factor = 0.0;
  }
  else
  {
    for (int i=0; i<num_exo_times-1; ++i)
    {
      if (time  <= (*_exodus_times)[i+1])
      {
        _exodus_index1 = i;
        _exodus_index2 = i+1;
        _interpolation_factor = (time-(*_exodus_times)[i])/((*_exodus_times)[i+1]-(*_exodus_times)[i]);
        break;
      }
      else if (i==num_exo_times-2)
      {
        _exodus_index1 = num_exo_times-1;
        _exodus_index2 = num_exo_times-1;
        _interpolation_factor = 1.0;
        break;
      }
    }
  }
  bool indices_modified(false);
  if (_exodus_index1 != old_index1 ||
      _exodus_index2 != old_index2)
  {
    indices_modified = true;
  }
  return indices_modified;
}

void
SolutionFunction::timestepSetup()
{
  if (_file_type==EXODUSII && _interpolate_times)
    updateExodusTimeInterpolation(_t);
}

Real
SolutionFunction::value(Real t, const Point & p)
{
  Real val = (*_mesh_function)(p);
  if (_file_type==EXODUSII && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,"Time passed into value() must match time at last call to timestepSetup()");
    Real val2 = (*_mesh_function2)(p);
    val = val + (val2 - val)*_interpolation_factor;
  }
  return val*_scale_factor;
}
