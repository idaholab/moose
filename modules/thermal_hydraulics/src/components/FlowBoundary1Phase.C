#include "FlowBoundary1Phase.h"
#include "FlowChannel1Phase.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"

InputParameters
FlowBoundary1Phase::validParams()
{
  InputParameters params = FlowConnection::validParams();
  params.addRequiredParam<BoundaryName>("input", "Name of the input");
  return params;
}

FlowBoundary1Phase::FlowBoundary1Phase(const InputParameters & params)
  : FlowConnection(params),
    _input(getParam<BoundaryName>("input")),
    _boundary_uo_name(genName(name(), "boundary_uo"))
{
  addConnection(_input);
  if (getConnections().size() > 0)
  {
    const Connection & connection = getConnections()[0];
    _connected_flow_channel_name = connection._geometrical_component_name;
    _connected_flow_channel_end_type = connection._end_type;
  }
}

void
FlowBoundary1Phase::setupMesh()
{
  FlowConnection::setupMesh();

  if (_nodes.size() > 0)
  {
    _node = _nodes[0];
    _normal = _normals[0];

    // create a nodeset/sideset corresponding to the node of the connected pipe end
    const BoundaryID boundary_id = _mesh.getNextBoundaryId();
    _mesh.getMesh().boundary_info->add_node(_node, boundary_id);
    _mesh.setBoundaryName(boundary_id, name());
  }
}

void
FlowBoundary1Phase::init()
{
  FlowConnection::init();

  if (hasComponentByName<FlowChannel1Phase>(_connected_flow_channel_name))
  {
    _numerical_flux_name = _numerical_flux_names[0];
    _rdg_int_var_uo_name = _rdg_int_var_uo_names[0];
  }
}

void
FlowBoundary1Phase::addWeakBC3Eqn()
{
  const std::string class_name = "ADBoundaryFlux3EqnBC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = getBoundaryNames();
  params.set<Real>("normal") = _normal;
  params.set<UserObjectName>("boundary_flux") = _boundary_uo_name;
  params.set<std::vector<VariableName>>("A_elem") = {FlowModel::AREA};
  params.set<std::vector<VariableName>>("A_linear") = {FlowModel::AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
  params.set<bool>("implicit") = _sim.getImplicitTimeIntegrationFlag();

  const std::vector<NonlinearVariableName> variables{
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};

  for (const auto & var : variables)
  {
    params.set<NonlinearVariableName>("variable") = var;
    _sim.addBoundaryCondition(class_name, genName(name(), var, "bnd_flux_3eqn_bc"), params);
  }
}
