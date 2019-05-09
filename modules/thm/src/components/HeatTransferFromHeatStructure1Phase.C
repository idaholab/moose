#include "HeatTransferFromHeatStructure1Phase.h"
#include "FlowChannel1Phase.h"
#include "HeatStructureBase.h"
#include "HeatStructureCylindrical.h"
#include "FlowModelSinglePhase.h"
#include "KDTree.h"
#include "THMMesh.h"
#include "libmesh/fe_interface.h"

registerMooseObject("THMApp", HeatTransferFromHeatStructure1Phase);

template <>
InputParameters
validParams<HeatTransferFromHeatStructure1Phase>()
{
  InputParameters params = validParams<HeatTransferFromTemperature1Phase>();
  params.addRequiredParam<std::string>("hs", "The name of the heat structure component");
  MooseEnum hs_sides("top bottom");
  params.addRequiredParam<MooseEnum>("hs_side", hs_sides, "The side of the heat structure");
  params.addClassDescription("Connects a 1-phase flow channel and a heat structure");
  return params;
}

HeatTransferFromHeatStructure1Phase::HeatTransferFromHeatStructure1Phase(
    const InputParameters & parameters)
  : HeatTransferFromTemperature1Phase(parameters),
    _hs_name(getParam<std::string>("hs")),
    _hs_side(getParam<MooseEnum>("hs_side"))
{
}

void
HeatTransferFromHeatStructure1Phase::checkFlowChannelAlignment() const
{
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  // master element centroids
  std::vector<Point> master_points;
  // element ids corresponding to the centroids in `master_points`
  std::vector<dof_id_type> master_elem_ids;
  // local side number corresponding to the element id in `master_elem_ids`
  std::vector<dof_id_type> master_elem_sides;
  // slave element centroids
  std::vector<Point> slave_points;
  // element ids corresponding to the centroids in `slave_points`
  std::vector<dof_id_type> slave_elem_ids;
  /// Map of the element ID and its nearest element ID
  std::map<dof_id_type, dof_id_type> nearest_elem_ids;
  /// Map of the element ID and local side number of the nearest element
  std::map<dof_id_type, unsigned int> nearest_elem_side;

  BoundaryID master_bnd_id = _mesh.getBoundaryID(getMasterSideName());
  BoundaryID slave_bnd_id = _mesh.getBoundaryID(getSlaveSideName());

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    BoundaryID boundary_id = belem->_bnd_id;

    if (boundary_id == master_bnd_id)
    {
      // 2D elements
      master_elem_ids.push_back(elem->id());
      master_elem_sides.push_back(belem->_side);
      master_points.push_back(elem->centroid());
      nearest_elem_side.insert(std::pair<dof_id_type, unsigned int>(elem->id(), belem->_side));
    }
    else if (boundary_id == slave_bnd_id)
    {
      if (std::find(slave_elem_ids.begin(), slave_elem_ids.end(), elem->id()) ==
          slave_elem_ids.end())
      {
        // 1D elements
        slave_elem_ids.push_back(elem->id());
        slave_points.push_back(elem->centroid());
      }
    }
  }

  if (master_points.size() > 0 && slave_points.size() > 0)
  {
    // find the master elements that are nearest to the slave elements
    KDTree kd_tree(master_points, _mesh.getMaxLeafSize());
    for (std::size_t i = 0; i < slave_points.size(); i++)
    {
      unsigned int patch_size = 1;
      std::vector<std::size_t> return_index(patch_size);
      kd_tree.neighborSearch(slave_points[i], patch_size, return_index);

      nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(slave_elem_ids[i], master_elem_ids[return_index[0]]));
      nearest_elem_ids.insert(
          std::pair<dof_id_type, dof_id_type>(master_elem_ids[return_index[0]], slave_elem_ids[i]));
    }

    // Go over all elements in the flow channel. Take the center of each element and project it onto
    // the heat structure side. Then check that the projected location on the heat structure matches
    // the location of the original center of the flow channel element
    const std::vector<unsigned int> & fch_elem_ids = flow_channel.getElementIDs();
    for (const auto & elem_id : fch_elem_ids)
    {
      const Elem * elem = _mesh.elemPtr(elem_id);
      Point center_pt = elem->centroid();

      const dof_id_type & hs_elem_id = nearest_elem_ids.at(elem_id);
      const unsigned int & hs_elem_side = nearest_elem_side.at(hs_elem_id);
      const Elem * neighbor = _mesh.elemPtr(hs_elem_id);
      const Elem * neighbor_side_elem = neighbor->build_side_ptr(hs_elem_side).release();
      unsigned int neighbor_dim = neighbor_side_elem->dim();
      Point ref_pt =
          FEInterface::inverse_map(neighbor_dim, FEType(), neighbor_side_elem, center_pt);
      Point hs_pt = FEInterface::map(neighbor_dim, FEType(), neighbor_side_elem, ref_pt);
      delete neighbor_side_elem;

      if (!center_pt.absolute_fuzzy_equals(hs_pt))
      {
        logError("The centers of the elements of flow channel '",
                 _flow_channel_name,
                 "' do not equal the centers of the specified heat structure side.");
        break;
      }
    }
  }
}

void
HeatTransferFromHeatStructure1Phase::check() const
{
  HeatTransferFromTemperature1Phase::check();

  checkComponentOfTypeExistsByName<HeatStructureBase>(_hs_name);

  if (hasComponentByName<HeatStructureBase>(_hs_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);

    const Real & P_hs = hs.getUnitPerimeter(_hs_side);
    if (MooseUtils::absoluteFuzzyEqual(P_hs, 0.))
      logError("'hs_side' parameter is set to '",
               _hs_side,
               "', but this side of the heat structure '",
               _hs_name,
               "' has radius of zero.");
  }

  if (hasComponentByName<HeatStructureBase>(_hs_name) &&
      hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);

    if (hs.getNumElems() != flow_channel.getNumElems())
      logError("The number of elements in component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getNumElems(),
               ", but the number of axial elements in component '",
               _hs_name,
               "' is ",
               hs.getNumElems(),
               ". They must be the same.");

    if (hs.getLength() != flow_channel.getLength())
      logError("The length of component '",
               _flow_channel_name,
               "' is ",
               flow_channel.getLength(),
               ", but the length of component '",
               _hs_name,
               "' is ",
               hs.getLength(),
               ". They must be the same.");

    checkFlowChannelAlignment();
  }
}

void
HeatTransferFromHeatStructure1Phase::addVariables()
{
  HeatTransferFromTemperature1Phase::addVariables();

  // wall temperature initial condition
  if (!_app.isRestarting())
  {
    const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
    _sim.addFunctionIC(_T_wall_name, hs.getInitialT(), _flow_channel_name);
  }
}

void
HeatTransferFromHeatStructure1Phase::addMooseObjects()
{
  HeatTransferFromTemperature1Phase::addMooseObjects();

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);
  const bool is_cylindrical = dynamic_cast<const HeatStructureCylindrical *>(&hs) != nullptr;
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  const UserObjectName heat_flux_uo_name = genName(name(), "heat_flux_uo");
  {
    const std::string class_name = "HeatFluxFromHeatStructure3EqnUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::vector<BoundaryName>>("slave_boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("master_boundary") = getMasterSideName();
    params.set<std::vector<VariableName>>("T_wall") = {_T_wall_name};
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<MaterialPropertyName>("Hw") = _Hw_1phase_name;
    params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<ExecFlagEnum>("execute_on") = execute_on;
    _sim.addUserObject(class_name, heat_flux_uo_name, params);
  }

  {
    const std::string class_name = "OneD3EqnEnergyHeatFlux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOEA;
    params.set<std::vector<VariableName>>("P_hf") = {_P_hf_name};
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    _sim.addKernel(class_name, genName(name(), "heat_flux_kernel"), params);
  }

  {
    const std::string class_name = "HeatFlux3EqnBC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<BoundaryName>>("boundary") = {getMasterSideName()};
    params.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    params.set<UserObjectName>("q_uo") = heat_flux_uo_name;
    params.set<Real>("P_hs_unit") = hs.getUnitPerimeter(_hs_side);
    params.set<unsigned int>("n_unit") = hs.getNumberOfUnits();
    params.set<bool>("hs_coord_system_is_cylindrical") = is_cylindrical;
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("T_wall") = {HeatConductionModel::TEMPERATURE};
    _sim.addBoundaryCondition(class_name, genName(name(), "heat_flux_bc"), params);
  }

  // Transfer the temperature of the solid onto the flow channel
  {
    std::string class_name = "VariableValueTransferAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = _T_wall_name;
    params.set<std::vector<BoundaryName>>("boundary") = {getSlaveSideName()};
    params.set<BoundaryName>("paired_boundary") = getMasterSideName();
    params.set<std::vector<VariableName>>("paired_variable") =
        std::vector<VariableName>(1, HeatConductionModel::TEMPERATURE);
    _sim.addAuxBoundaryCondition(class_name, genName(name(), "T_wall_transfer"), params);
  }
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getMasterSideName() const
{
  const HeatStructureBase & hs = getComponentByName<HeatStructureBase>(_hs_name);

  switch (_hs_side)
  {
    case 0:
      if (hs.getTopBoundaryNames().size() > 0)
        return hs.getTopBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    case 1:
      if (hs.getBottomBoundaryNames().size() > 0)
        return hs.getBottomBoundaryNames()[0];
      else
        return THMMesh::INVALID_BOUNDARY_ID;

    default:
      mooseError(name(), ": Unknown side specified in the 'hs_side' parameter.");
  }
}

const BoundaryName &
HeatTransferFromHeatStructure1Phase::getSlaveSideName() const
{
  const FlowChannel1Phase & flow_channel =
      getComponentByName<FlowChannel1Phase>(_flow_channel_name);
  return flow_channel.getNodesetName();
}
