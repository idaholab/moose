// MOOSE includes
#include "MooseError.h"
#include "SolutionUserObject.h"

// libMesh includes
#include "MooseMesh.h"
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/parallel_mesh.h"

template<>
InputParameters validParams<SolutionUserObject>()
{
  // Get the input parameters from the parent class
  InputParameters params = validParams<UserObject>();

  // Add required parameters
  params.addRequiredParam<std::string>("mesh", "The name of the mesh file (must be xda or exodusII file.");
  params.addRequiredParam<std::vector<std::string> >("variables", "The name of the variable from the file you want to use for values.");

  // When using XDA files the following must be defined
  params.addParam<std::string>("es", "The name of the file holding the equation system info in xda format (xda only).");
  params.addParam<std::string>("system", "NonlinearSystem", "The name of the system to pull values out of (xda only).");

  // When using ExodusII a specific time is extracted
  params.addParam<int>("timestep", -1, "Index of the single timestep used (exodusII only).  If not supplied, time interpolation will occur.");

  // Re-set the default exeuction time, due to the how this class interacts with SolutionAux and SolutionFunction, this
  // must be set to timestep_begin to interp values in Exodus files correctly.
  params.set<MooseEnum>("execute_on") = "timestep_begin";

  // Return the parameters
  return params;
}

SolutionUserObject::SolutionUserObject(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _file_type(MooseEnum("xda=0, exodusII=1")),
    _mesh_file(getParam<std::string>("mesh")),
    _es_file(getParam<std::string>("es")),
    _system_name(getParam<std::string>("system")),
    _var_name(getParam<std::vector<std::string> >("variables")),
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
    _exodus_index2(-1)
{

  // Create a libmesh::Mesh object for storing the loaded data.  This _should_
  // work regardless of whether the underlying type of Mesh is SerialMesh or
  // ParallelMesh (if libmesh has been compiled with --enable-parmesh or not),
  // so this line is OK.
  _mesh = new Mesh;

  // ExodusII mesh file supplied
  if (MooseUtils::hasExtension(_mesh_file, "e"))
  {
    _file_type = "exodusII";
    readExodusII();
  }

  // XDA mesh file supplied
  else if (MooseUtils::hasExtension(_mesh_file, "xda"))
  {
    _file_type = "xda";
    readXda();
  }

  // Produce an error for an unknown file type
  else
    mooseError("In SolutionUserObject, invalid file type (only .xda and .e supported)");

  // Intilize the serial solution vector
  _serialized_solution = NumericVector<Number>::build().release();
  _serialized_solution->init(_system->n_dofs(), false, SERIAL);

  // Pull down a full copy of this vector on every processor so we can get values in parallel
  _system->solution->localize(*_serialized_solution);

  // Gather the variable numbers for the desired variables
  _var_num.resize(_var_name.size());
  for (unsigned int i = 0; i < _var_num.size(); ++i)
    _var_num[i] = _system->variable_number(_var_name[i]);

  // Create the MeshFunction for working with the solution data
  _mesh_function = new MeshFunction(*_es, *_serialized_solution, _system->get_dof_map(), _var_num);
  _mesh_function->init();
}

SolutionUserObject::~SolutionUserObject()
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

void
SolutionUserObject::readXda()
{
  // Check that the EquationSystems XDA file
  if (_es_file == "")
      mooseError("In SolutionUserObject, es must be supplied when file_type=xda");

  // Check that a system name is defined, for extraction from the file
  if (_system_name == "")
      mooseError("In SolutionUserObject, system must be supplied when file_type=xda");

  // Read the libmesh::mesh from the xda file
  _mesh->read(_mesh_file);

  // Create, read, and update the libmesh::EquationSystems
  _es = new EquationSystems(*_mesh);
  _es->read(_es_file);
  _es->update();

  // Store the EquationSystems name locally
  _system = &_es->get_system(_system_name);
}

void
SolutionUserObject::readExodusII()
{
  // Define a default system name
  if (_system_name == "")
    _system_name = "SolutionUserObjectSystem";

  // Interpolate between times rather than using values from a set timestep
  if (_exodus_time_index == -1)
    _interpolate_times = true;  // Read the file

  // Read the Exodus file
  _exodusII_io = new ExodusII_IO (*_mesh);
  _exodusII_io->read(_mesh_file);
  _exodus_times = &_exodusII_io->get_time_steps();

  // Check that the number of time steps is valid
  int num_exo_times = _exodus_times->size();
  if (num_exo_times == 0)
    mooseError("In SolutionUserObject, exodus file contains no timesteps.");

  // Account for parallel mesh
  if (dynamic_cast<ParallelMesh *>(_mesh))
  {
    _mesh->allow_renumbering(true);
    _mesh->prepare_for_use(/*false*/);
  }
  else
  {
    _mesh->allow_renumbering(false);
    _mesh->prepare_for_use(/*true*/);
  }

  // Create EquationSystems object for solution
  _es = new EquationSystems(*_mesh);
  _es->add_system<ExplicitSystem> (_system_name);
  _system = &_es->get_system(_system_name);

  // Add the variables to the system
  for (unsigned int i = 0; i < _var_name.size(); ++i)
  {
    std::cout << "Adding variable " << _var_name[i] << std::cout;
    _system->add_variable(_var_name[i], FIRST);
  }

  // Initilize the equations systems
  _es->init();

  // Interpolate times
  if (_interpolate_times)
  {
    // Create a second equation system
    _es2 = new EquationSystems(*_mesh);
    _es2->add_system<ExplicitSystem> (_system_name);
    _system2 = &_es2->get_system(_system_name);

    // Add the variables
    for (unsigned int i = 0; i < _var_name.size(); ++i)
      _system2->add_variable(_var_name[i], FIRST);

    // Initialize
    _es2->init();

    // Update the times for interpolation (initially start at 0)
    updateExodusBracketingTimeIndices(0.0);

    // Copy the nodal solution to the equations systems from the Exodus file
    for (unsigned int i = 0; i < _var_name.size(); ++i)
    {
      _exodusII_io->copy_nodal_solution(*_system, _var_name[i], _exodus_index1+1);
      _exodusII_io->copy_nodal_solution(*_system2, _var_name[i], _exodus_index2+1);
    }

    // Update the systems
    _system->update();
    _es->update();
    _system2->update();
    _es2->update();

    // Populate variable numbers for the second system
    std::vector<unsigned int> var_num2;
    _system2->get_all_variable_numbers(var_num2);

    // Need to pull down a full copy of this vector on every processor so we can get values in parallel
    _serialized_solution2 = NumericVector<Number>::build().release();
    _serialized_solution2->init(_system2->n_dofs(), false, SERIAL);
    _system2->solution->localize(*_serialized_solution2);

    // Create the MeshFunction for the second copy of the data
    _mesh_function2 = new MeshFunction(*_es2, *_serialized_solution2, _system2->get_dof_map(), var_num2);
    _mesh_function2->init();
  }

  // Non-interpolated times
  else
  {
    if (_exodus_time_index > num_exo_times)
      mooseError("In SolutionUserObject, timestep = "<<_exodus_time_index<<", but there are only "<<num_exo_times<<" time steps.");

    // Copy the values from the ExodusII file
    for (unsigned int i = 0; i < _var_name.size(); ++i)
      _exodusII_io->copy_nodal_solution(*_system, _var_name[i], _exodus_time_index);

    // Update the equations systems
    _system->update();
    _es->update();
  }
}

Real
SolutionUserObject::directValue(const Node * node, const std::string & var_name) const
{
  // Get the libmesh variable and system numbers
  unsigned int var_num = _system->variable_number(var_name);
  unsigned int sys_num = _system->number();

  // Get the node id and associated dof
  unsigned int node_id = node->id();
  unsigned int dof_id = _fe_problem.mesh().node(node_id).dof_number(sys_num, var_num, 0);

  // Return the desried value for the dof
  return directValue(dof_id);
}

Real
SolutionUserObject::directValue(const Elem * elem, const std::string & var_name) const
{
  // Get the libmesh variable and system numbers
  unsigned int var_num = _system->variable_number(var_name);
  unsigned int sys_num = _system->number();

  // Get the element id and associated dof
  unsigned int elem_id = elem->id();
  unsigned int dof_id = _fe_problem.mesh().elem(elem_id)->dof_number(sys_num, var_num, 0);

  // Return the desired value
  return directValue(dof_id);
}

void
SolutionUserObject::initialize()
{
}

void
SolutionUserObject::finalize()
{
}

void
SolutionUserObject::timestepSetup()
{
  // Update time interpolatation for ExodusII solution
  if (_file_type == 1 && _interpolate_times)
    updateExodusTimeInterpolation(_t);
}

void
SolutionUserObject::execute()
{
}

MooseEnum
SolutionUserObject::getSolutionFileType()
{
  return _file_type;
}

void
SolutionUserObject::updateExodusTimeInterpolation(Real time)
{
  if (time != _interpolation_time)
  {
    if (updateExodusBracketingTimeIndices(time))
    {

      for (unsigned int i = 0; i < _var_name.size(); ++i)
        _exodusII_io->copy_nodal_solution(*_system, _var_name[i], _exodus_index1+1);

      _system->update();
      _es->update();
      _system->solution->localize(*_serialized_solution);

      for (unsigned int i = 0; i < _var_name.size(); ++i)
        _exodusII_io->copy_nodal_solution(*_system2, _var_name[i], _exodus_index2+1);

      _system2->update();
      _es2->update();
      _system2->solution->localize(*_serialized_solution2);
    }
    _interpolation_time = time;
  }
}

bool
SolutionUserObject::updateExodusBracketingTimeIndices(Real time)
{
  if (_file_type != 1)
    mooseError("In SolutionUserObject, getTimeInterpolationData only applicable for exodusII file type");

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


Real
SolutionUserObject::pointValue(Real t, const Point & p, const std::string & var_name) const
{
  // Extract the value at the current point
  Real val = evalMeshFunction(p, var_name, 1);

  // Interplolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,"Time passed into value() must match time at last call to timestepSetup()");
    Real val2 = evalMeshFunction(p, var_name, 2);
    val = val + (val2 - val)*_interpolation_factor;
  }
  return val;
}

Real
SolutionUserObject::directValue(unsigned int dof_index) const
{
  Real val = (*_serialized_solution)(dof_index);
  if (_file_type==1 && _interpolate_times)
  {
    Real val2 = (*_serialized_solution2)(dof_index);
    val = val + (val2 - val)*_interpolation_factor;
  }
  return val;
}

Real
SolutionUserObject::evalMeshFunction(const Point & p, std::string var_name, unsigned int func_num) const
{
  // Storage for mesh function output
  DenseVector<Number> output;

  // Extract a value from the _mesh_function
  if (func_num == 1)
  {
    (*_mesh_function)(p, 0.0, output);
    return output(_system->variable_number(var_name));
  }

  // Extract a value from _mesh_function2
  else if (func_num == 2)
  {
    (*_mesh_function2)(p, 0.0, output);
    return output(_system2->variable_number(var_name));
  }

  else
    mooseError("The func_num must be 1 or 2");
}
