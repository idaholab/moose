//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionUserObject.h"

// MOOSE includes
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseVariableFE.h"
#include "RotationMatrix.h"

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/mesh_function.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/transient_system.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"
#include "libmesh/enum_xdr_mode.h"

registerMooseObject("MooseApp", SolutionUserObject);

InputParameters
SolutionUserObject::validParams()
{
  // Get the input parameters from the parent class
  InputParameters params = GeneralUserObject::validParams();

  // Add required parameters
  params.addRequiredParam<MeshFileName>(
      "mesh", "The name of the mesh file (must be xda or exodusII file).");
  params.addParam<std::vector<std::string>>(
      "system_variables",
      std::vector<std::string>(),
      "The name of the nodal and elemental variables from the file you want to use for values");

  // When using XDA files the following must be defined
  params.addParam<FileName>(
      "es",
      "<not supplied>",
      "The name of the file holding the equation system info in xda format (xda only).");
  params.addParam<std::string>(
      "system", "nl0", "The name of the system to pull values out of (xda only).");

  // When using ExodusII a specific time is extracted
  params.addParam<std::string>("timestep",
                               "Index of the single timestep used or \"LATEST\" for "
                               "the last timestep (exodusII only).  If not supplied, "
                               "time interpolation will occur.");

  // Add ability to perform coordinate transformation: scale, factor
  params.addParam<std::vector<Real>>(
      "scale", std::vector<Real>(LIBMESH_DIM, 1), "Scale factor for points in the simulation");
  params.addParam<std::vector<Real>>("scale_multiplier",
                                     std::vector<Real>(LIBMESH_DIM, 1),
                                     "Scale multiplying factor for points in the simulation");
  params.addParam<std::vector<Real>>("translation",
                                     std::vector<Real>(LIBMESH_DIM, 0),
                                     "Translation factors for x,y,z coordinates of the simulation");
  params.addParam<RealVectorValue>("rotation0_vector",
                                   RealVectorValue(0, 0, 1),
                                   "Vector about which to rotate points of the simulation.");
  params.addParam<Real>(
      "rotation0_angle",
      0.0,
      "Anticlockwise rotation angle (in degrees) to use for rotation about rotation0_vector.");
  params.addParam<RealVectorValue>("rotation1_vector",
                                   RealVectorValue(0, 0, 1),
                                   "Vector about which to rotate points of the simulation.");
  params.addParam<Real>(
      "rotation1_angle",
      0.0,
      "Anticlockwise rotation angle (in degrees) to use for rotation about rotation1_vector.");

  // following lines build the default_transformation_order
  MultiMooseEnum default_transformation_order(
      "rotation0 translation scale rotation1 scale_multiplier", "translation scale");
  params.addParam<MultiMooseEnum>(
      "transformation_order",
      default_transformation_order,
      "The order to perform the operations in.  Define R0 to be the rotation matrix encoded by "
      "rotation0_vector and rotation0_angle.  Similarly for R1.  Denote the scale by s, the "
      "scale_multiplier by m, and the translation by t.  Then, given a point x in the simulation, "
      "if transformation_order = 'rotation0 scale_multiplier translation scale rotation1' then "
      "form p = R1*(R0*x*m - t)/s.  Then the values provided by the SolutionUserObject at point x "
      "in the simulation are the variable values at point p in the mesh.");
  params.addClassDescription("Reads a variable from a mesh in one simulation to another");
  // Return the parameters
  return params;
}

// Static mutex definition
Threads::spin_mutex SolutionUserObject::_solution_user_object_mutex;

SolutionUserObject::SolutionUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _file_type(MooseEnum("xda=0 exodusII=1 xdr=2")),
    _mesh_file(getParam<MeshFileName>("mesh")),
    _es_file(getParam<FileName>("es")),
    _system_name(getParam<std::string>("system")),
    _system_variables(getParam<std::vector<std::string>>("system_variables")),
    _exodus_time_index(-1),
    _interpolate_times(false),
    _system(nullptr),
    _system2(nullptr),
    _interpolation_time(0.0),
    _interpolation_factor(0.0),
    _exodus_times(nullptr),
    _exodus_index1(-1),
    _exodus_index2(-1),
    _scale(getParam<std::vector<Real>>("scale")),
    _scale_multiplier(getParam<std::vector<Real>>("scale_multiplier")),
    _translation(getParam<std::vector<Real>>("translation")),
    _rotation0_vector(getParam<RealVectorValue>("rotation0_vector")),
    _rotation0_angle(getParam<Real>("rotation0_angle")),
    _r0(RealTensorValue()),
    _rotation1_vector(getParam<RealVectorValue>("rotation1_vector")),
    _rotation1_angle(getParam<Real>("rotation1_angle")),
    _r1(RealTensorValue()),
    _transformation_order(getParam<MultiMooseEnum>("transformation_order")),
    _initialized(false)
{
  // form rotation matrices with the specified angles
  Real halfPi = std::acos(0.0);
  Real a;
  Real b;

  a = std::cos(halfPi * -_rotation0_angle / 90);
  b = std::sin(halfPi * -_rotation0_angle / 90);
  // the following is an anticlockwise rotation about z
  RealTensorValue rot0_z(a, -b, 0, b, a, 0, 0, 0, 1);
  // form the rotation matrix that will take rotation0_vector to the z axis
  RealTensorValue vec0_to_z = RotationMatrix::rotVecToZ(_rotation0_vector);
  // _r0 is then: rotate points so vec0 lies along z; then rotate about angle0; then rotate points
  // back
  _r0 = vec0_to_z.transpose() * (rot0_z * vec0_to_z);

  a = std::cos(halfPi * -_rotation1_angle / 90);
  b = std::sin(halfPi * -_rotation1_angle / 90);
  // the following is an anticlockwise rotation about z
  RealTensorValue rot1_z(a, -b, 0, b, a, 0, 0, 0, 1);
  // form the rotation matrix that will take rotation1_vector to the z axis
  RealTensorValue vec1_to_z = RotationMatrix::rotVecToZ(_rotation1_vector);
  // _r1 is then: rotate points so vec1 lies along z; then rotate about angle1; then rotate points
  // back
  _r1 = vec1_to_z.transpose() * (rot1_z * vec1_to_z);

  if (isParamValid("timestep") && getParam<std::string>("timestep") == "-1")
    mooseError("A \"timestep\" of -1 is no longer supported for interpolation. Instead simply "
               "remove this parameter altogether for interpolation");
}

SolutionUserObject::~SolutionUserObject() {}

void
SolutionUserObject::readXda()
{
  // Check that the required files exist
  MooseUtils::checkFileReadable(_es_file);
  MooseUtils::checkFileReadable(_mesh_file);

  // Read the libmesh::mesh from the xda file
  _mesh->read(_mesh_file);

  // Create the libmesh::EquationSystems
  _es = std::make_unique<EquationSystems>(*_mesh);

  // Use new read syntax (binary)
  if (_file_type == "xdr")
    _es->read(_es_file,
              DECODE,
              EquationSystems::READ_HEADER | EquationSystems::READ_DATA |
                  EquationSystems::READ_ADDITIONAL_DATA);

  // Use new read syntax
  else if (_file_type == "xda")
    _es->read(_es_file,
              READ,
              EquationSystems::READ_HEADER | EquationSystems::READ_DATA |
                  EquationSystems::READ_ADDITIONAL_DATA);

  // This should never occur, just in case produce an error
  else
    mooseError("Failed to determine proper read method for XDA/XDR equation system file: ",
               _es_file);

  // Update and store the EquationSystems name locally
  _es->update();
  _system = &_es->get_system(_system_name);
}

void
SolutionUserObject::readExodusII()
{
  // Define a default system name
  if (_system_name == "")
    _system_name = "SolutionUserObjectSystem";

  // Read the Exodus file
  _exodusII_io = std::make_unique<ExodusII_IO>(*_mesh);
  _exodusII_io->read(_mesh_file);
  readBlockIdMapFromExodusII();
  _exodus_times = &_exodusII_io->get_time_steps();

  if (isParamValid("timestep"))
  {
    std::string s_timestep = getParam<std::string>("timestep");
    int n_steps = _exodusII_io->get_num_time_steps();
    if (s_timestep == "LATEST")
      _exodus_time_index = n_steps;
    else
    {
      std::istringstream ss(s_timestep);
      if (!((ss >> _exodus_time_index) && ss.eof()) || _exodus_time_index > n_steps)
        mooseError("Invalid value passed as \"timestep\". Expected \"LATEST\" or a valid integer "
                   "less than ",
                   n_steps,
                   ", received ",
                   s_timestep);
    }
  }
  else
    // Interpolate between times rather than using values from a set timestep
    _interpolate_times = true;

  // Check that the number of time steps is valid
  int num_exo_times = _exodus_times->size();
  if (num_exo_times == 0)
    mooseError("In SolutionUserObject, exodus file contains no timesteps.");

  // Account for parallel mesh
  if (dynamic_cast<DistributedMesh *>(_mesh.get()))
  {
    _mesh->allow_renumbering(true);
    _mesh->prepare_for_use();
  }
  else
  {
    _mesh->allow_renumbering(false);
    _mesh->prepare_for_use();
  }

  // Create EquationSystems object for solution
  _es = std::make_unique<EquationSystems>(*_mesh);
  _es->add_system<ExplicitSystem>(_system_name);
  _system = &_es->get_system(_system_name);

  // Get the variable name lists as set; these need to be sets to perform set_intersection
  const std::vector<std::string> & all_nodal(_exodusII_io->get_nodal_var_names());
  const std::vector<std::string> & all_elemental(_exodusII_io->get_elem_var_names());
  const std::vector<std::string> & all_scalar(_exodusII_io->get_global_var_names());

  // Build nodal/elemental variable lists, limit to variables listed in 'system_variables', if
  // provided
  if (!_system_variables.empty())
  {
    for (const auto & var_name : _system_variables)
    {
      if (std::find(all_nodal.begin(), all_nodal.end(), var_name) != all_nodal.end())
        _nodal_variables.push_back(var_name);
      if (std::find(all_elemental.begin(), all_elemental.end(), var_name) != all_elemental.end())
        _elemental_variables.push_back(var_name);
      if (std::find(all_scalar.begin(), all_scalar.end(), var_name) != all_scalar.end())
        // Check if the scalar matches any field variables, and ignore the var if it does. This
        // means its a Postprocessor.
        if (std::find(begin(_nodal_variables), end(_nodal_variables), var_name) ==
                _nodal_variables.end() &&
            std::find(begin(_elemental_variables), end(_elemental_variables), var_name) ==
                _elemental_variables.end())
          _scalar_variables.push_back(var_name);
    }
  }
  else
  {
    _nodal_variables = all_nodal;
    _elemental_variables = all_elemental;

    for (auto var_name : all_scalar)
      // Check if the scalar matches any field variables, and ignore the var if it does. This means
      // its a Postprocessor.
      if (std::find(begin(_nodal_variables), end(_nodal_variables), var_name) ==
              _nodal_variables.end() &&
          std::find(begin(_elemental_variables), end(_elemental_variables), var_name) ==
              _elemental_variables.end())
        _scalar_variables.push_back(var_name);
  }

  // Add the variables to the system
  for (const auto & var_name : _nodal_variables)
    _system->add_variable(var_name, FIRST);

  for (const auto & var_name : _elemental_variables)
    _system->add_variable(var_name, CONSTANT, MONOMIAL);

  for (const auto & var_name : _scalar_variables)
    _system->add_variable(var_name, FIRST, SCALAR);

  // Initialize the equations systems
  _es->init();

  // Interpolate times
  if (_interpolate_times)
  {
    // Create a second equation system
    _es2 = std::make_unique<EquationSystems>(*_mesh);
    _es2->add_system<ExplicitSystem>(_system_name);
    _system2 = &_es2->get_system(_system_name);

    // Add the variables to the system
    for (const auto & var_name : _nodal_variables)
      _system2->add_variable(var_name, FIRST);

    for (const auto & var_name : _elemental_variables)
      _system2->add_variable(var_name, CONSTANT, MONOMIAL);

    for (const auto & var_name : _scalar_variables)
      _system2->add_variable(var_name, FIRST, SCALAR);

    // Initialize
    _es2->init();

    // Update the times for interpolation (initially start at 0)
    updateExodusBracketingTimeIndices(0.0);

    // Copy the solutions from the first system
    for (const auto & var_name : _nodal_variables)
    {
      _exodusII_io->copy_nodal_solution(*_system, var_name, var_name, _exodus_index1 + 1);
      _exodusII_io->copy_nodal_solution(*_system2, var_name, var_name, _exodus_index2 + 1);
    }

    for (const auto & var_name : _elemental_variables)
    {
      _exodusII_io->copy_elemental_solution(*_system, var_name, var_name, _exodus_index1 + 1);
      _exodusII_io->copy_elemental_solution(*_system2, var_name, var_name, _exodus_index2 + 1);
    }

    if (_scalar_variables.size() > 0)
    {
      _exodusII_io->copy_scalar_solution(
          *_system, _scalar_variables, _scalar_variables, _exodus_index1 + 1);
      _exodusII_io->copy_scalar_solution(
          *_system2, _scalar_variables, _scalar_variables, _exodus_index2 + 1);
    }

    // Update the systems
    _system->update();
    _es->update();
    _system2->update();
    _es2->update();
  }

  // Non-interpolated times
  else
  {
    if (_exodus_time_index > num_exo_times)
      mooseError("In SolutionUserObject, timestep = ",
                 _exodus_time_index,
                 ", but there are only ",
                 num_exo_times,
                 " time steps.");

    // Copy the values from the ExodusII file
    for (const auto & var_name : _nodal_variables)
      _exodusII_io->copy_nodal_solution(*_system, var_name, var_name, _exodus_time_index);

    for (const auto & var_name : _elemental_variables)
      _exodusII_io->copy_elemental_solution(*_system, var_name, var_name, _exodus_time_index);

    if (!_scalar_variables.empty())
      _exodusII_io->copy_scalar_solution(
          *_system, _scalar_variables, _scalar_variables, _exodus_time_index);

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
  dof_id_type node_id = node->id();
  const Node & sys_node = _system->get_mesh().node_ref(node_id);
  mooseAssert(sys_node.n_dofs(sys_num, var_num) > 0,
              "Variable " << var_name << " has no DoFs on node " << sys_node.id());
  dof_id_type dof_id = sys_node.dof_number(sys_num, var_num, 0);

  // Return the desired value for the dof
  return directValue(dof_id);
}

Real
SolutionUserObject::directValue(const Elem * elem, const std::string & var_name) const
{
  // Get the libmesh variable and system numbers
  unsigned int var_num = _system->variable_number(var_name);
  unsigned int sys_num = _system->number();

  // Get the element id and associated dof
  dof_id_type elem_id = elem->id();
  const Elem & sys_elem = _system->get_mesh().elem_ref(elem_id);
  mooseAssert(sys_elem.n_dofs(sys_num, var_num) > 0,
              "Variable " << var_name << " has no DoFs on element " << sys_elem.id());
  dof_id_type dof_id = sys_elem.dof_number(sys_num, var_num, 0);

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
  // Update time interpolation for ExodusII solution
  if (_file_type == 1 && _interpolate_times)
    updateExodusTimeInterpolation(_t);
}

void
SolutionUserObject::execute()
{
}

void
SolutionUserObject::initialSetup()
{

  // Make sure this only happens once
  if (_initialized)
    return;

  // Create a libmesh::Mesh object for storing the loaded data.
  // Several aspects of SolutionUserObject won't work with a DistributedMesh:
  // .) ExodusII_IO::copy_nodal_solution() doesn't work in parallel.
  // .) We don't know if directValue will be used, which may request
  //    a value on a Node we don't have.
  // We force the Mesh used here to be a ReplicatedMesh.
  _mesh = std::make_unique<ReplicatedMesh>(_communicator);

  // ExodusII mesh file supplied
  if (MooseUtils::hasExtension(_mesh_file, "e", /*strip_exodus_ext =*/true))
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

  else if (MooseUtils::hasExtension(_mesh_file, "xdr"))
  {
    _file_type = "xdr";
    readXda();
  }

  // Produce an error for an unknown file type
  else
    mooseError("In SolutionUserObject, invalid file type (only .xda, .xdr, and .e supported)");

  // Initialize the serial solution vector
  _serialized_solution = NumericVector<Number>::build(_communicator);
  _serialized_solution->init(_system->n_dofs(), false, SERIAL);

  // Pull down a full copy of this vector on every processor so we can get values in parallel
  _system->solution->localize(*_serialized_solution);

  // Vector of variable numbers to apply the MeshFunction to
  std::vector<unsigned int> var_nums;

  // If no variables were given, use all of them
  if (_system_variables.empty())
  {
    _system->get_all_variable_numbers(var_nums);
    for (const auto & var_num : var_nums)
      _system_variables.push_back(_system->variable_name(var_num));
  }

  // Otherwise, gather the numbers for the variables given
  else
  {
    for (const auto & var_name : _system_variables)
      var_nums.push_back(_system->variable_number(var_name));
  }

  // Create the MeshFunction for working with the solution data
  _mesh_function =
      std::make_unique<MeshFunction>(*_es, *_serialized_solution, _system->get_dof_map(), var_nums);
  _mesh_function->init();

  // Tell the MeshFunctions that we might be querying them outside the
  // mesh, so we can handle any errors at the MOOSE rather than at the
  // libMesh level.
  DenseVector<Number> default_values;
  _mesh_function->enable_out_of_mesh_mode(default_values);

  // Build second MeshFunction for interpolation
  if (_interpolate_times)
  {
    // Need to pull down a full copy of this vector on every processor so we can get values in
    // parallel
    _serialized_solution2 = NumericVector<Number>::build(_communicator);
    _serialized_solution2->init(_system2->n_dofs(), false, SERIAL);
    _system2->solution->localize(*_serialized_solution2);

    // Create the MeshFunction for the second copy of the data
    _mesh_function2 = std::make_unique<MeshFunction>(
        *_es2, *_serialized_solution2, _system2->get_dof_map(), var_nums);
    _mesh_function2->init();
    _mesh_function2->enable_out_of_mesh_mode(default_values);
  }

  // Populate the MeshFunction variable index
  for (unsigned int i = 0; i < _system_variables.size(); ++i)
  {
    std::string name = _system_variables[i];
    _local_variable_index[name] = i;
  }

  // Set initialization flag
  _initialized = true;
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

      for (const auto & var_name : _nodal_variables)
        _exodusII_io->copy_nodal_solution(*_system, var_name, var_name, _exodus_index1 + 1);
      for (const auto & var_name : _elemental_variables)
        _exodusII_io->copy_elemental_solution(*_system, var_name, var_name, _exodus_index1 + 1);
      if (_scalar_variables.size() > 0)
        _exodusII_io->copy_scalar_solution(
            *_system, _scalar_variables, _scalar_variables, _exodus_index1 + 1);

      _system->update();
      _es->update();
      _system->solution->localize(*_serialized_solution);

      for (const auto & var_name : _nodal_variables)
        _exodusII_io->copy_nodal_solution(*_system2, var_name, var_name, _exodus_index2 + 1);
      for (const auto & var_name : _elemental_variables)
        _exodusII_io->copy_elemental_solution(*_system2, var_name, var_name, _exodus_index2 + 1);
      if (_scalar_variables.size() > 0)
        _exodusII_io->copy_scalar_solution(
            *_system2, _scalar_variables, _scalar_variables, _exodus_index2 + 1);

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
    mooseError(
        "In SolutionUserObject, getTimeInterpolationData only applicable for exodusII file type");

  int old_index1 = _exodus_index1;
  int old_index2 = _exodus_index2;

  int num_exo_times = _exodus_times->size();

  if (time < (*_exodus_times)[0])
  {
    _exodus_index1 = 0;
    _exodus_index2 = 0;
    _interpolation_factor = 0.0;
  }
  else
  {
    for (int i = 0; i < num_exo_times - 1; ++i)
    {
      if (time <= (*_exodus_times)[i + 1])
      {
        _exodus_index1 = i;
        _exodus_index2 = i + 1;
        _interpolation_factor =
            (time - (*_exodus_times)[i]) / ((*_exodus_times)[i + 1] - (*_exodus_times)[i]);
        break;
      }
      else if (i == num_exo_times - 2)
      {
        _exodus_index1 = num_exo_times - 1;
        _exodus_index2 = num_exo_times - 1;
        _interpolation_factor = 1.0;
        break;
      }
    }
  }

  bool indices_modified(false);

  if (_exodus_index1 != old_index1 || _exodus_index2 != old_index2)
    indices_modified = true;

  return indices_modified;
}

unsigned int
SolutionUserObject::getLocalVarIndex(const std::string & var_name) const
{
  // Extract the variable index for the MeshFunction(s)
  std::map<std::string, unsigned int>::const_iterator it = _local_variable_index.find(var_name);
  if (it == _local_variable_index.end())
    mooseError("Value requested for nonexistent variable '",
               var_name,
               "' in the '",
               name(),
               "' SolutionUserObject");
  return it->second;
}

Real
SolutionUserObject::pointValueWrapper(Real t,
                                      const Point & p,
                                      const std::string & var_name,
                                      const MooseEnum & weighting_type,
                                      const std::set<subdomain_id_type> * subdomain_ids) const
{
  // first check if the FE type is continuous because in that case the value is
  // unique and we can take a short cut, the default weighting_type found_first also
  // shortcuts out
  auto family =
      _fe_problem
          .getVariable(
              _tid, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD)
          .feType()
          .family;

  if (weighting_type == 1 ||
      (family != L2_LAGRANGE && family != MONOMIAL && family != L2_HIERARCHIC))
    return pointValue(t, p, var_name, subdomain_ids);

  // the shape function is discontinuous so we need to compute a suitable unique value
  std::map<const Elem *, Real> values = discontinuousPointValue(t, p, var_name);
  switch (weighting_type)
  {
    case 2:
    {
      Real average = 0.0;
      for (auto & v : values)
        average += v.second;
      return average / Real(values.size());
    }
    case 4:
    {
      Real smallest_elem_id_value = std::numeric_limits<Real>::max();
      dof_id_type smallest_elem_id = _fe_problem.mesh().maxElemId();
      for (auto & v : values)
        if (v.first->id() < smallest_elem_id)
        {
          smallest_elem_id = v.first->id();
          smallest_elem_id_value = v.second;
        }
      return smallest_elem_id_value;
    }
    case 8:
    {
      Real largest_elem_id_value = std::numeric_limits<Real>::lowest();
      dof_id_type largest_elem_id = 0;
      for (auto & v : values)
        if (v.first->id() > largest_elem_id)
        {
          largest_elem_id = v.first->id();
          largest_elem_id_value = v.second;
        }
      return largest_elem_id_value;
    }
  }

  mooseError(
      "SolutionUserObject::pointValueWrapper reaches line that it should not be able to reach.");
  return 0.0;
}

Real
SolutionUserObject::pointValue(Real t,
                               const Point & p,
                               const std::string & var_name,
                               const std::set<subdomain_id_type> * subdomain_ids) const
{
  const unsigned int local_var_index = getLocalVarIndex(var_name);
  return pointValue(t, p, local_var_index, subdomain_ids);
}

Real
SolutionUserObject::pointValue(Real libmesh_dbg_var(t),
                               const Point & p,
                               const unsigned int local_var_index,
                               const std::set<subdomain_id_type> * subdomain_ids) const
{
  // Create copy of point
  Point pt(p);

  // do the transformations
  for (unsigned int trans_num = 0; trans_num < _transformation_order.size(); ++trans_num)
  {
    if (_transformation_order[trans_num] == "rotation0")
      pt = _r0 * pt;
    else if (_transformation_order[trans_num] == "translation")
      for (const auto i : make_range(Moose::dim))
        pt(i) -= _translation[i];
    else if (_transformation_order[trans_num] == "scale")
      for (const auto i : make_range(Moose::dim))
        pt(i) /= _scale[i];
    else if (_transformation_order[trans_num] == "scale_multiplier")
      for (const auto i : make_range(Moose::dim))
        pt(i) *= _scale_multiplier[i];
    else if (_transformation_order[trans_num] == "rotation1")
      pt = _r1 * pt;
  }

  // Extract the value at the current point
  Real val = evalMeshFunction(pt, local_var_index, 1, subdomain_ids);

  // Interpolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,
                "Time passed into value() must match time at last call to timestepSetup()");
    Real val2 = evalMeshFunction(pt, local_var_index, 2, subdomain_ids);
    val = val + (val2 - val) * _interpolation_factor;
  }

  return val;
}

std::map<const Elem *, Real>
SolutionUserObject::discontinuousPointValue(Real t,
                                            const Point & p,
                                            const std::string & var_name,
                                            const std::set<subdomain_id_type> * subdomain_ids) const
{
  const unsigned int local_var_index = getLocalVarIndex(var_name);
  return discontinuousPointValue(t, p, local_var_index, subdomain_ids);
}

std::map<const Elem *, Real>
SolutionUserObject::discontinuousPointValue(Real libmesh_dbg_var(t),
                                            Point pt,
                                            const unsigned int local_var_index,
                                            const std::set<subdomain_id_type> * subdomain_ids) const
{
  // do the transformations
  for (unsigned int trans_num = 0; trans_num < _transformation_order.size(); ++trans_num)
  {
    if (_transformation_order[trans_num] == "rotation0")
      pt = _r0 * pt;
    else if (_transformation_order[trans_num] == "translation")
      for (const auto i : make_range(Moose::dim))
        pt(i) -= _translation[i];
    else if (_transformation_order[trans_num] == "scale")
      for (const auto i : make_range(Moose::dim))
        pt(i) /= _scale[i];
    else if (_transformation_order[trans_num] == "scale_multiplier")
      for (const auto i : make_range(Moose::dim))
        pt(i) *= _scale_multiplier[i];
    else if (_transformation_order[trans_num] == "rotation1")
      pt = _r1 * pt;
  }

  // Extract the value at the current point
  std::map<const Elem *, Real> map =
      evalMultiValuedMeshFunction(pt, local_var_index, 1, subdomain_ids);

  // Interpolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,
                "Time passed into value() must match time at last call to timestepSetup()");
    std::map<const Elem *, Real> map2 = evalMultiValuedMeshFunction(pt, local_var_index, 2);

    if (map.size() != map2.size())
      mooseError("In SolutionUserObject::discontinuousPointValue map and map2 have different size");

    // construct the interpolated map
    for (auto & k : map)
    {
      if (map2.find(k.first) == map2.end())
        mooseError(
            "In SolutionUserObject::discontinuousPointValue map and map2 have differing keys");
      Real val = k.second;
      Real val2 = map2[k.first];
      map[k.first] = val + (val2 - val) * _interpolation_factor;
    }
  }

  return map;
}

RealGradient
SolutionUserObject::pointValueGradientWrapper(
    Real t,
    const Point & p,
    const std::string & var_name,
    const MooseEnum & weighting_type,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  // the default weighting_type found_first shortcuts out
  if (weighting_type == 1)
    return pointValueGradient(t, p, var_name, subdomain_ids);

  // the shape function is discontinuous so we need to compute a suitable unique value
  std::map<const Elem *, RealGradient> values =
      discontinuousPointValueGradient(t, p, var_name, subdomain_ids);
  switch (weighting_type)
  {
    case 2:
    {
      RealGradient average = RealGradient(0.0, 0.0, 0.0);
      for (auto & v : values)
        average += v.second;
      return average / Real(values.size());
    }
    case 4:
    {
      RealGradient smallest_elem_id_value;
      dof_id_type smallest_elem_id = _fe_problem.mesh().maxElemId();
      for (auto & v : values)
        if (v.first->id() < smallest_elem_id)
        {
          smallest_elem_id = v.first->id();
          smallest_elem_id_value = v.second;
        }
      return smallest_elem_id_value;
    }
    case 8:
    {
      RealGradient largest_elem_id_value;
      dof_id_type largest_elem_id = 0;
      for (auto & v : values)
        if (v.first->id() > largest_elem_id)
        {
          largest_elem_id = v.first->id();
          largest_elem_id_value = v.second;
        }
      return largest_elem_id_value;
    }
  }

  mooseError("SolutionUserObject::pointValueGradientWrapper reaches line that it should not be "
             "able to reach.");
  return RealGradient(0.0, 0.0, 0.0);
}

RealGradient
SolutionUserObject::pointValueGradient(Real t,
                                       const Point & p,
                                       const std::string & var_name,
                                       const std::set<subdomain_id_type> * subdomain_ids) const
{
  const unsigned int local_var_index = getLocalVarIndex(var_name);
  return pointValueGradient(t, p, local_var_index, subdomain_ids);
}

RealGradient
SolutionUserObject::pointValueGradient(Real libmesh_dbg_var(t),
                                       Point pt,
                                       const unsigned int local_var_index,
                                       const std::set<subdomain_id_type> * subdomain_ids) const
{
  // do the transformations
  for (unsigned int trans_num = 0; trans_num < _transformation_order.size(); ++trans_num)
  {
    if (_transformation_order[trans_num] == "rotation0")
      pt = _r0 * pt;
    else if (_transformation_order[trans_num] == "translation")
      for (const auto i : make_range(Moose::dim))
        pt(i) -= _translation[i];
    else if (_transformation_order[trans_num] == "scale")
      for (const auto i : make_range(Moose::dim))
        pt(i) /= _scale[i];
    else if (_transformation_order[trans_num] == "scale_multiplier")
      for (const auto i : make_range(Moose::dim))
        pt(i) *= _scale_multiplier[i];
    else if (_transformation_order[trans_num] == "rotation1")
      pt = _r1 * pt;
  }

  // Extract the value at the current point
  RealGradient val = evalMeshFunctionGradient(pt, local_var_index, 1, subdomain_ids);

  // Interpolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,
                "Time passed into value() must match time at last call to timestepSetup()");
    RealGradient val2 = evalMeshFunctionGradient(pt, local_var_index, 2, subdomain_ids);
    val = val + (val2 - val) * _interpolation_factor;
  }

  return val;
}

std::map<const Elem *, RealGradient>
SolutionUserObject::discontinuousPointValueGradient(
    Real t,
    const Point & p,
    const std::string & var_name,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  const unsigned int local_var_index = getLocalVarIndex(var_name);
  return discontinuousPointValueGradient(t, p, local_var_index, subdomain_ids);
}

std::map<const Elem *, RealGradient>
SolutionUserObject::discontinuousPointValueGradient(
    Real libmesh_dbg_var(t),
    Point pt,
    const unsigned int local_var_index,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  // do the transformations
  for (unsigned int trans_num = 0; trans_num < _transformation_order.size(); ++trans_num)
  {
    if (_transformation_order[trans_num] == "rotation0")
      pt = _r0 * pt;
    else if (_transformation_order[trans_num] == "translation")
      for (const auto i : make_range(Moose::dim))
        pt(i) -= _translation[i];
    else if (_transformation_order[trans_num] == "scale")
      for (const auto i : make_range(Moose::dim))
        pt(i) /= _scale[i];
    else if (_transformation_order[trans_num] == "scale_multiplier")
      for (const auto i : make_range(Moose::dim))
        pt(i) *= _scale_multiplier[i];
    else if (_transformation_order[trans_num] == "rotation1")
      pt = _r1 * pt;
  }

  // Extract the value at the current point
  std::map<const Elem *, RealGradient> map =
      evalMultiValuedMeshFunctionGradient(pt, local_var_index, 1, subdomain_ids);

  // Interpolate
  if (_file_type == 1 && _interpolate_times)
  {
    mooseAssert(t == _interpolation_time,
                "Time passed into value() must match time at last call to timestepSetup()");
    std::map<const Elem *, RealGradient> map2 =
        evalMultiValuedMeshFunctionGradient(pt, local_var_index, 1, subdomain_ids);

    if (map.size() != map2.size())
      mooseError("In SolutionUserObject::discontinuousPointValue map and map2 have different size");

    // construct the interpolated map
    for (auto & k : map)
    {
      if (map2.find(k.first) == map2.end())
        mooseError(
            "In SolutionUserObject::discontinuousPointValue map and map2 have differing keys");
      RealGradient val = k.second;
      RealGradient val2 = map2[k.first];
      map[k.first] = val + (val2 - val) * _interpolation_factor;
    }
  }

  return map;
}

Real
SolutionUserObject::directValue(dof_id_type dof_index) const
{
  Real val = (*_serialized_solution)(dof_index);
  if (_file_type == 1 && _interpolate_times)
  {
    Real val2 = (*_serialized_solution2)(dof_index);
    val = val + (val2 - val) * _interpolation_factor;
  }
  return val;
}

Real
SolutionUserObject::evalMeshFunction(const Point & p,
                                     const unsigned int local_var_index,
                                     unsigned int func_num,
                                     const std::set<subdomain_id_type> * subdomain_ids) const
{
  // Storage for mesh function output
  DenseVector<Number> output;

  // Extract a value from the _mesh_function
  {
    Threads::spin_mutex::scoped_lock lock(_solution_user_object_mutex);
    if (func_num == 1)
      (*_mesh_function)(p, 0.0, output, subdomain_ids);

    // Extract a value from _mesh_function2
    else if (func_num == 2)
      (*_mesh_function2)(p, 0.0, output, subdomain_ids);

    else
      mooseError("The func_num must be 1 or 2");
  }

  // Error if the data is out-of-range, which will be the case if the mesh functions are evaluated
  // outside the domain
  if (output.size() == 0)
  {
    std::ostringstream oss;
    p.print(oss);
    mooseError("Failed to access the data for variable '",
               _system_variables[local_var_index],
               "' at point ",
               oss.str(),
               " in the '",
               name(),
               "' SolutionUserObject");
  }
  return output(local_var_index);
}

std::map<const Elem *, Real>
SolutionUserObject::evalMultiValuedMeshFunction(
    const Point & p,
    const unsigned int local_var_index,
    unsigned int func_num,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  // Storage for mesh function output
  std::map<const Elem *, DenseVector<Number>> temporary_output;

  // Extract a value from the _mesh_function
  {
    Threads::spin_mutex::scoped_lock lock(_solution_user_object_mutex);
    if (func_num == 1)
      _mesh_function->discontinuous_value(p, 0.0, temporary_output, subdomain_ids);

    // Extract a value from _mesh_function2
    else if (func_num == 2)
      _mesh_function2->discontinuous_value(p, 0.0, temporary_output, subdomain_ids);

    else
      mooseError("The func_num must be 1 or 2");
  }

  // Error if the data is out-of-range, which will be the case if the mesh functions are evaluated
  // outside the domain
  if (temporary_output.size() == 0)
  {
    std::ostringstream oss;
    p.print(oss);
    mooseError("Failed to access the data for variable '",
               _system_variables[local_var_index],
               "' at point ",
               oss.str(),
               " in the '",
               name(),
               "' SolutionUserObject");
  }

  // Fill the actual map that is returned
  std::map<const Elem *, Real> output;
  for (auto & k : temporary_output)
  {
    mooseAssert(k.second.size() > local_var_index,
                "In SolutionUserObject::evalMultiValuedMeshFunction variable with local_var_index "
                    << local_var_index << " does not exist");
    output[k.first] = k.second(local_var_index);
  }

  return output;
}

RealGradient
SolutionUserObject::evalMeshFunctionGradient(
    const Point & p,
    const unsigned int local_var_index,
    unsigned int func_num,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  // Storage for mesh function output
  std::vector<Gradient> output;

  // Extract a value from the _mesh_function
  {
    Threads::spin_mutex::scoped_lock lock(_solution_user_object_mutex);
    if (func_num == 1)
      _mesh_function->gradient(p, 0.0, output, subdomain_ids);

    // Extract a value from _mesh_function2
    else if (func_num == 2)
      _mesh_function2->gradient(p, 0.0, output, subdomain_ids);

    else
      mooseError("The func_num must be 1 or 2");
  }

  // Error if the data is out-of-range, which will be the case if the mesh functions are evaluated
  // outside the domain
  if (output.size() == 0)
  {
    std::ostringstream oss;
    p.print(oss);
    mooseError("Failed to access the data for variable '",
               _system_variables[local_var_index],
               "' at point ",
               oss.str(),
               " in the '",
               name(),
               "' SolutionUserObject");
  }
  return output[local_var_index];
}

std::map<const Elem *, RealGradient>
SolutionUserObject::evalMultiValuedMeshFunctionGradient(
    const Point & p,
    const unsigned int local_var_index,
    unsigned int func_num,
    const std::set<subdomain_id_type> * subdomain_ids) const
{
  // Storage for mesh function output
  std::map<const Elem *, std::vector<Gradient>> temporary_output;

  // Extract a value from the _mesh_function
  {
    Threads::spin_mutex::scoped_lock lock(_solution_user_object_mutex);
    if (func_num == 1)
      _mesh_function->discontinuous_gradient(p, 0.0, temporary_output, subdomain_ids);

    // Extract a value from _mesh_function2
    else if (func_num == 2)
      _mesh_function2->discontinuous_gradient(p, 0.0, temporary_output, subdomain_ids);

    else
      mooseError("The func_num must be 1 or 2");
  }

  // Error if the data is out-of-range, which will be the case if the mesh functions are evaluated
  // outside the domain
  if (temporary_output.size() == 0)
  {
    std::ostringstream oss;
    p.print(oss);
    mooseError("Failed to access the data for variable '",
               _system_variables[local_var_index],
               "' at point ",
               oss.str(),
               " in the '",
               name(),
               "' SolutionUserObject");
  }

  // Fill the actual map that is returned
  std::map<const Elem *, RealGradient> output;
  for (auto & k : temporary_output)
  {
    mooseAssert(k.second.size() > local_var_index,
                "In SolutionUserObject::evalMultiValuedMeshFunction variable with local_var_index "
                    << local_var_index << " does not exist");
    output[k.first] = k.second[local_var_index];
  }

  return output;
}

const std::vector<std::string> &
SolutionUserObject::variableNames() const
{
  return _system_variables;
}

bool
SolutionUserObject::isVariableNodal(const std::string & var_name) const
{
  return std::find(_nodal_variables.begin(), _nodal_variables.end(), var_name) !=
         _nodal_variables.end();
}

Real
SolutionUserObject::scalarValue(Real /*t*/, const std::string & var_name) const
{
  unsigned int var_num = _system->variable_number(var_name);
  const DofMap & dof_map = _system->get_dof_map();
  std::vector<dof_id_type> dofs;
  dof_map.SCALAR_dof_indices(dofs, var_num);
  // We can handle only FIRST order scalar variables
  return directValue(dofs[0]);
}

void
SolutionUserObject::readBlockIdMapFromExodusII()
{
#ifdef LIBMESH_HAVE_EXODUS_API
  ExodusII_IO_Helper & exio_helper = _exodusII_io->get_exio_helper();
  std::map<int, std::string> & id_to_block = exio_helper.id_to_block_names;
  _block_name_to_id.clear();
  for (auto & it : id_to_block)
    _block_name_to_id[it.second] = it.first;
#endif
}
