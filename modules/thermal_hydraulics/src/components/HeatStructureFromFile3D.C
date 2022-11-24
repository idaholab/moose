//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureFromFile3D.h"
#include "THMMesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureFromFile3D);

InputParameters
HeatStructureFromFile3D::validParams()
{
  InputParameters params = HeatStructureBase::validParams();
  params.addRequiredParam<FileName>("file", "The file name with the mesh (in Exodus format)");
  params.addParam<FunctionName>("initial_T", "Initial temperature [K]");
  // arbitrary non-zero vector
  params.set<RealVectorValue>("orientation") = RealVectorValue(1, 0, 0);
  params.suppressParameter<RealVectorValue>("orientation");
  params.suppressParameter<std::vector<std::string>>("axial_region_names");
  params.set<std::vector<Real>>("length") = {1};
  params.suppressParameter<std::vector<Real>>("length");
  params.set<std::vector<unsigned int>>("n_elems") = {0};
  params.suppressParameter<std::vector<unsigned int>>("n_elems");
  params.set<std::vector<Real>>("widths") = {1};
  params.suppressParameter<std::vector<Real>>("widths");
  params.set<std::vector<unsigned int>>("n_part_elems") = {1};
  params.suppressParameter<std::vector<unsigned int>>("n_part_elems");
  params.addClassDescription(
      "Heat structure component that can load the mesh from an ExodusII file");
  return params;
}

HeatStructureFromFile3D::HeatStructureFromFile3D(const InputParameters & params)
  : HeatStructureBase(params), _file_name(getParam<FileName>("file"))
{
  _num_rods = 1;

  if (MooseUtils::pathExists(_file_name) && MooseUtils::checkFileReadable(_file_name, false, false))
  {
    ExodusII_IO_Helper exio_helper(*this, false, true, false);
    exio_helper.open(_file_name.c_str(), true);
    exio_helper.read_and_store_header_info();
    if (exio_helper.num_dim != 3)
      logError("File '", _file_name, "' does not contain a 3D mesh.");
  }
  else
    logError("Could not open '",
             _file_name,
             "'. Check that the file exists and that you have permission to open it.");
}

std::shared_ptr<HeatConductionModel>
HeatStructureFromFile3D::buildModel()
{
  const std::string class_name = "HeatConductionModel";
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<THMProblem *>("_thm_problem") = &_sim;
  pars.set<HeatStructureBase *>("_hs") = this;
  pars.applyParameters(parameters());
  return _factory.create<HeatConductionModel>(class_name, name(), pars, 0);
}

void
HeatStructureFromFile3D::init()
{
  _hc_model = buildModel();
}

void
HeatStructureFromFile3D::check() const
{
  GeometricalComponent::check();

  bool ics_set = _sim.hasInitialConditionsFromFile() || isParamValid("initial_T");
  if (!ics_set && !_app.isRestarting())
    logError("Missing initial condition for temperature.");
}

bool
HeatStructureFromFile3D::usingSecondOrderMesh() const
{
  return false;
}

void
HeatStructureFromFile3D::buildMesh()
{
  if (!MooseUtils::pathExists(_file_name) ||
      !MooseUtils::checkFileReadable(_file_name, false, false))
    return;

  ExodusII_IO_Helper exio_helper(*this, false, true, false);

  exio_helper.open(_file_name.c_str(), true);

  exio_helper.read_and_store_header_info();

  // node map from exodus id to THM id
  std::map<int, unsigned int> thm_node_map;
  // element map from exodus id to THM id
  std::map<int, unsigned int> thm_elem_map;
  // boundary condition map from exodus id to THM id
  std::map<int, unsigned int> thm_bc_id_map;

  // nodes
  exio_helper.read_nodes();
  exio_helper.read_node_num_map();
  for (int i = 0; i < exio_helper.num_nodes; i++)
  {
    int exodus_id = exio_helper.node_num_map[i];

    Point p(exio_helper.x[i], exio_helper.y[i], exio_helper.z[i]);
    const Node * nd = addNode(p);
    thm_node_map[exodus_id] = nd->id();
  }

  // blocks
  exio_helper.read_block_info();
  exio_helper.read_elem_num_map();
  int nelem_last_block = 0;
  for (int i = 0; i < exio_helper.num_elem_blk; i++)
  {
    exio_helper.read_elem_in_block(i);
    int blk_id = exio_helper.get_block_id(i);

    std::string subdomain_name = exio_helper.get_block_name(i);
    if (subdomain_name.empty())
      subdomain_name = Moose::stringify(blk_id);
    _names.push_back(subdomain_name);
    _name_index[subdomain_name] = i;

    const std::string solid_block_name = genName(_name, subdomain_name);
    SubdomainID sid = _mesh.getNextSubdomainId();
    setSubdomainInfo(sid, solid_block_name, Moose::COORD_XYZ);

    const std::string type_str(exio_helper.get_elem_type());
    const auto & conv = exio_helper.get_conversion(type_str);

    int jmax = nelem_last_block + exio_helper.num_elem_this_blk;
    for (int j = nelem_last_block; j < jmax; j++)
    {
      std::vector<dof_id_type> node_ids(exio_helper.num_nodes_per_elem);
      for (int k = 0; k < exio_helper.num_nodes_per_elem; k++)
      {
        int gi = (j - nelem_last_block) * exio_helper.num_nodes_per_elem + conv.get_node_map(k);
        int ex_node_id = exio_helper.node_num_map[exio_helper.connect[gi] - 1];
        node_ids[k] = thm_node_map[ex_node_id];
      }
      Elem * elem = addElement(conv.libmesh_elem_type(), node_ids);
      elem->subdomain_id() = sid;

      int exodus_id = exio_helper.elem_num_map[j];
      thm_elem_map[exodus_id] = elem->id();
    }
    nelem_last_block += exio_helper.num_elem_this_blk;
  }

  // build side sets
  exio_helper.read_sideset_info();
  int offset = 0;
  std::unordered_map<BoundaryID, BoundaryName> new_ids_to_names;
  for (int i = 0; i < exio_helper.num_side_sets; i++)
  {
    // Compute new offset
    offset += (i > 0 ? exio_helper.num_sides_per_set[i - 1] : 0);
    exio_helper.read_sideset(i, offset);

    int ex_sideset_id = exio_helper.get_side_set_id(i);
    std::string sideset_name = exio_helper.get_side_set_name(i);
    if (sideset_name.empty())
      sideset_name = Moose::stringify(ex_sideset_id);

    unsigned int bc_id = _mesh.getNextBoundaryId();
    thm_bc_id_map[ex_sideset_id] = bc_id;
    new_ids_to_names.emplace(bc_id, sideset_name);
  }

  auto & boundary_info = _mesh.getMesh().get_boundary_info();

  for (auto e : index_range(exio_helper.elem_list))
  {
    int ex_elem_id = exio_helper.elem_num_map[exio_helper.elem_list[e] - 1];
    dof_id_type elem_id = thm_elem_map[ex_elem_id];
    Elem * elem = _mesh.elemPtr(elem_id);
    const auto & conv = exio_helper.get_conversion(elem->type());
    // Map the zero-based Exodus side numbering to the libmesh side numbering
    unsigned int raw_side_index = exio_helper.side_list[e] - 1;
    std::size_t side_index_offset = conv.get_shellface_index_offset();
    unsigned int side_index = static_cast<unsigned int>(raw_side_index - side_index_offset);
    int mapped_side = conv.get_side_map(side_index);
    unsigned int bc_id = thm_bc_id_map[exio_helper.id_list[e]];
    boundary_info.add_side(elem, mapped_side, bc_id);
  }
  for (const auto & pr : new_ids_to_names)
    _mesh.getMesh().get_boundary_info().sideset_name(pr.first) = genName(_name, pr.second);

  _number_of_hs = _names.size();
}

void
HeatStructureFromFile3D::addVariables()
{
  _hc_model->addVariables();
  if (isParamValid("initial_T"))
    _hc_model->addInitialConditions();
}

void
HeatStructureFromFile3D::addMooseObjects()
{
  HeatStructureBase::addMooseObjects();
  _hc_model->addHeatEquationXYZ();
}

FunctionName
HeatStructureFromFile3D::getInitialT() const
{
  if (isParamValid("initial_T"))
    return getParam<FunctionName>("initial_T");
  else
    mooseError(name(), ": The parameter 'initial_T' was requested but not supplied");
}

Real
HeatStructureFromFile3D::getUnitPerimeter(const HeatStructureSideType & /*side*/) const
{
  // Unit perimeter of 1 means that heat sources and heat flux from heat structure won't be scaled
  // (which is what we want)
  return 1.;
}
