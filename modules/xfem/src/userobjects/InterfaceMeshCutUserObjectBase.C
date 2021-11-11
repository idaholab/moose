//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMeshCutUserObjectBase.h"
#include "XFEMMovingInterfaceVelocityBase.h"

InputParameters
InterfaceMeshCutUserObjectBase::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>("mesh_file", "Mesh file for the XFEM geometric cut.");
  params.addParam<UserObjectName>("interface_velocity_uo",
                                  "The name of userobject that computes the velocity.");
  params.addParam<FunctionName>("interface_velocity_function",
                                "The name of function that provides interface velocity.");
  params.addParam<bool>("output_exodus", true, "Whether to output exodus file of cutter mesh.");
  params.addParam<CutSubdomainID>(
      "negative_id", 0, "The CutSubdomainID corresponding to a non-positive signed distance");
  params.addParam<CutSubdomainID>(
      "positive_id", 1, "The CutSubdomainID corresponding to a positive signed distance");
  return params;
}

InterfaceMeshCutUserObjectBase::InterfaceMeshCutUserObjectBase(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _interface_velocity(nullptr),
    _func(nullptr),
    _mesh(_subproblem.mesh()),
    _exodus_io(nullptr),
    _explicit_system(nullptr),
    _equation_systems(nullptr),
    _output_exodus(getParam<bool>("output_exodus")),
    _negative_id(getParam<CutSubdomainID>("negative_id")),
    _positive_id(getParam<CutSubdomainID>("positive_id"))
{
  MeshFileName xfem_cutter_mesh_file = getParam<MeshFileName>("mesh_file");
  _cutter_mesh = std::make_shared<ReplicatedMesh>(_communicator);
  _cutter_mesh->read(xfem_cutter_mesh_file);
  _exodus_io = std::make_unique<ExodusII_IO>(*_cutter_mesh);

  if (!parameters.isParamSetByUser("interface_velocity_uo") &&
      !parameters.isParamSetByUser("interface_velocity_function"))
    paramError("Either `interface_velocity_uo` or `interface_velocity_function` must be provided.");
  else if (parameters.isParamSetByUser("interface_velocity_uo") &&
           parameters.isParamSetByUser("interface_velocity_function"))
    paramError(
        "'interface_velocity_uo` and `interface_velocity_function` cannot be both provided.");

  if (parameters.isParamSetByUser("interface_velocity_function"))
    _func = &getFunction("interface_velocity_function");
}

void
InterfaceMeshCutUserObjectBase::initialSetup()
{
  if (_func == nullptr)
  {
    const UserObject * uo =
        &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("interface_velocity_uo")));

    if (dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo) == nullptr)
      mooseError("UserObject casting to XFEMMovingInterfaceVelocityBase in "
                 "MovingLineSegmentCutSetUserObject");

    _interface_velocity = dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo);
    const_cast<XFEMMovingInterfaceVelocityBase *>(_interface_velocity)->initialize();
  }

  for (const auto & node : _cutter_mesh->node_ptr_range())
    _initial_nodes_location[node->id()] = *node;

  for (const auto & elem : _cutter_mesh->element_ptr_range())
    for (unsigned int n = 0; n < elem->n_nodes(); n++)
      _node_to_elem_map[elem->node_id(n)].push_back(elem->id());

  _cutter_mesh->prepare_for_use();
  _cutter_mesh->set_mesh_dimension(_mesh.dimension() - 1);

  if (_output_exodus)
  {
    _equation_systems = std::make_unique<EquationSystems>(*_cutter_mesh);
    _explicit_system = &(_equation_systems->add_system<ExplicitSystem>("InterfaceMeshSystem"));

    _explicit_system->add_variable("disp_x");
    _explicit_system->add_variable("disp_y");

    if (_mesh.dimension() == 3)
      _explicit_system->add_variable("disp_z");

    _equation_systems->init();
    _exodus_io->write_equation_systems(_app.getOutputFileBase() + "_" + name() + ".e",
                                       *_equation_systems);

    _var_num_disp_x = _explicit_system->variable_number("disp_x");
    _var_num_disp_y = _explicit_system->variable_number("disp_y");
    if (_mesh.dimension() == 3)
      _var_num_disp_z = _explicit_system->variable_number("disp_z");
  }
}

void
InterfaceMeshCutUserObjectBase::initialize()
{
  std::vector<Point> new_position(_cutter_mesh->n_nodes());

  _pl = _mesh.getPointLocator();
  _pl->enable_out_of_mesh_mode();

  std::map<unsigned int, Real> node_velocity;
  Real sum = 0.0;
  unsigned count = 0;
  // Loop all the nodes to calculate the velocity
  for (const auto & node : _cutter_mesh->node_ptr_range())
  {
    if ((*_pl)(*node) != nullptr)
    {
      Real velocity;
      if (_func == nullptr)
        velocity =
            _interface_velocity->computeMovingInterfaceVelocity(node->id(), nodeNormal(node->id()));
      else
        velocity = _func->value(_t, *node);

      // only updates when t_step >0
      if (_t_step <= 0)
        velocity = 0.0;

      node_velocity[node->id()] = velocity;
      sum += velocity;
      count++;
    }
  }

  if (count == 0)
    mooseError("No node of the cutter mesh is found inside the computational domain.");

  Real average_velocity = sum / count;

  for (const auto & node : _cutter_mesh->node_ptr_range())
  {
    if ((*_pl)(*node) == nullptr)
      node_velocity[node->id()] = average_velocity;
  }

  for (const auto & node : _cutter_mesh->node_ptr_range())
  {
    Point p = *node;
    p += _dt * nodeNormal(node->id()) * node_velocity[node->id()];
    new_position[node->id()] = p;
  }
  for (const auto & node : _cutter_mesh->node_ptr_range())
    _cutter_mesh->node_ref(node->id()) = new_position[node->id()];

  if (_output_exodus)
  {
    std::vector<dof_id_type> di;
    for (const auto & node : _cutter_mesh->node_ptr_range())
    {
      _explicit_system->get_dof_map().dof_indices(node, di, _var_num_disp_x);
      _explicit_system->solution->set(
          di[0], new_position[node->id()](0) - _initial_nodes_location[node->id()](0));

      _explicit_system->get_dof_map().dof_indices(node, di, _var_num_disp_y);
      _explicit_system->solution->set(
          di[0], new_position[node->id()](1) - _initial_nodes_location[node->id()](1));

      if (_mesh.dimension() == 3)
      {
        _explicit_system->get_dof_map().dof_indices(node, di, _var_num_disp_z);
        _explicit_system->solution->set(
            di[0], new_position[node->id()](2) - _initial_nodes_location[node->id()](2));
      }
    }

    _explicit_system->solution->close();

    _exodus_io->append(true);
    _exodus_io->write_timestep(
        _app.getOutputFileBase() + "_" + name() + ".e", *_equation_systems, _t_step + 1, _t);
  }

  calculateNormals();
}

const std::vector<Point>
InterfaceMeshCutUserObjectBase::getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for this object.");
}

const std::vector<RealVectorValue>
InterfaceMeshCutUserObjectBase::getCrackPlaneNormals(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for this object.");
}

CutSubdomainID
InterfaceMeshCutUserObjectBase::getCutSubdomainID(const Node * node) const
{
  return calculateSignedDistance(*node) > 0.0 ? _positive_id : _negative_id;
}
