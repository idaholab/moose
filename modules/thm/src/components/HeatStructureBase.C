#include "HeatStructureBase.h"
#include "SolidMaterialProperties.h"
#include "ConstantFunction.h"
#include "Enums.h"
#include "THMMesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad9.h"

const std::map<std::string, HeatStructureBase::SideType> HeatStructureBase::_side_type_to_enum{
    {"INNER", INNER}, {"OUTER", OUTER}};

MooseEnum
HeatStructureBase::getSideType(const std::string & name)
{
  return THM::getMooseEnum<SideType>(name, _side_type_to_enum);
}

template <>
HeatStructureBase::SideType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<HeatStructureBase::SideType>(s, HeatStructureBase::_side_type_to_enum);
}

template <>
InputParameters
validParams<HeatStructureBase>()
{
  InputParameters params = validParams<GeometricalComponent>();
  params.addPrivateParam<std::string>("component_type", "heat_struct");
  params.addParam<FunctionName>("initial_T", "Initial temperature");
  params.addRequiredParam<std::vector<std::string>>("names", "User given heat structure names");
  params.addRequiredParam<std::vector<Real>>("widths", "Width of each heat structure");
  params.addRequiredParam<std::vector<unsigned int>>("n_part_elems",
                                                     "Number of elements of each heat structure");
  params.addRequiredParam<std::vector<std::string>>("materials",
                                                    "Material names to be used in heat structures");
  params.addParam<Real>("num_rods", 1., "The number of rods represented by this heat structure.");
  return params;
}

HeatStructureBase::HeatStructureBase(const InputParameters & params)
  : GeometricalComponent(params),
    _number_of_hs(0),
    _names(getParam<std::vector<std::string>>("names")),
    _material_names(getParam<std::vector<std::string>>("materials")),
    _width(getParam<std::vector<Real>>("widths")),
    _total_width(std::accumulate(_width.begin(), _width.end(), 0.0)),
    _n_part_elems(getParam<std::vector<unsigned int>>("n_part_elems")),
    _total_elem_number(0),
    _num_rods(getParam<Real>("num_rods")),
    _has_k(params.isParamValid("k")),
    _has_Cp(params.isParamValid("Cp")),
    _has_rho(params.isParamValid("rho"))
{
  _number_of_hs = _names.size();
  if (_n_part_elems.size() == _number_of_hs)
  {
    for (unsigned int i = 0; i < _number_of_hs; i++)
      _total_elem_number += _n_part_elems[i];
  }

  for (unsigned int i = 0; i < _names.size(); i++)
    _name_index[_names[i]] = i;
}

std::shared_ptr<HeatConductionModel>
HeatStructureBase::buildModel()
{
  const std::string class_name = "HeatConductionModel";
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<HeatStructureBase *>("_hs") = this;
  return _factory.create<HeatConductionModel>(class_name, name(), pars, 0);
}

void
HeatStructureBase::init()
{
  _hc_model = buildModel();
}

void
HeatStructureBase::check() const
{
  GeometricalComponent::check();

  if (!isParamValid("initial_T") && !_app.isRestarting())
    logError("Missing initial condition for temperature.");
}

bool
HeatStructureBase::hasBlock(const std::string & name) const
{
  for (unsigned int i = 0; i < _names.size(); i++)
    if (_names[i] == name)
      return true;
  return false;
}

const unsigned int &
HeatStructureBase::getIndexFromName(const std::string & name) const
{
  return _name_index.at(name);
}

bool
HeatStructureBase::usingSecondOrderMesh() const
{
  return HeatConductionModel::feType().order == SECOND;
}

Real
HeatStructureBase::getAxialOffset() const
{
  return 0.;
}

void
HeatStructureBase::build2DMesh()
{
  // loop on flow channel nodes to create heat structure nodes
  unsigned int n_layers = _node_locations.size();
  std::vector<std::vector<unsigned int>> node_ids(
      n_layers, std::vector<unsigned int>(_total_elem_number + 1));

  // loop over layers
  for (unsigned int i = 0; i < n_layers; i++)
  {
    Point p(_node_locations[i], 0, 0);
    p(1) += getAxialOffset();

    Node * nd = addNode(p);
    node_ids[i][0] = nd->id();

    // loop over all heat structures
    unsigned int l = 1;
    for (unsigned int j = 0; j < _number_of_hs; j++)
    {
      Real elem_length = _width[j] / _n_part_elems[j];
      for (unsigned int k = 0; k < _n_part_elems[j]; k++, l++)
      {
        p(1) += elem_length;
        nd = addNode(p);
        node_ids[i][l] = nd->id();
      }
      _side_heat_node_ids[_names[j]].push_back(nd->id());
    }
    _inner_heat_node_ids.push_back(node_ids[i][0]);
    _outer_heat_node_ids.push_back(node_ids[i][_total_elem_number]);
  }

  // create elements from nodes
  unsigned int bc_id1 = getNextBoundaryId();
  unsigned int bc_id2 = getNextBoundaryId();
  _inner_bc_id.push_back(bc_id1);
  _outer_bc_id.push_back(bc_id2);

  _elem_ids.resize(_n_elem);
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    unsigned int n = 0;
    for (unsigned int j = 0; j < _number_of_hs; j++) // loop on all heat structures to add elements
    {
      for (unsigned int k = 0; k < _n_part_elems[j]; k++)
      {
        Elem * elem = _mesh.getMesh().add_elem(new Quad4);
        elem->subdomain_id() = _subdomain_ids[j];
        elem->set_node(0) = _mesh.getMesh().node_ptr(node_ids[i][n + 1]);
        elem->set_node(1) = _mesh.getMesh().node_ptr(node_ids[i][n]);
        elem->set_node(2) = _mesh.getMesh().node_ptr(node_ids[i + 1][n]);
        elem->set_node(3) = _mesh.getMesh().node_ptr(node_ids[i + 1][n + 1]);

        _elem_ids[i].push_back(elem->id());

        if (n == 0)
          _mesh.getMesh().boundary_info->add_side(elem, 1, bc_id1);
        if (n == _total_elem_number - 1)
          _mesh.getMesh().boundary_info->add_side(elem, 3, bc_id2);

        n++;
      }
    }
  }
}

void
HeatStructureBase::build2DMesh2ndOrder()
{
  // loop on flow channel nodes to create heat structure nodes
  unsigned int n_layers = _node_locations.size();
  std::vector<std::vector<unsigned int>> node_ids(
      n_layers, std::vector<unsigned int>(2 * _total_elem_number + 1));

  // loop over layers
  for (unsigned int i = 0; i < n_layers; i++)
  {
    Point p(_node_locations[i], 0, 0);
    p(1) += getAxialOffset();

    const Node * nd = addNode(p);
    node_ids[i][0] = nd->id();

    // loop over all heat structures
    unsigned int l = 1;
    for (unsigned int j = 0; j < _number_of_hs; j++)
    {
      Real elem_length = _width[j] / (2. * _n_part_elems[j]);
      for (unsigned int k = 0; k < 2. * _n_part_elems[j]; k++, l++)
      {
        p(1) += elem_length;
        nd = addNode(p);
        node_ids[i][l] = nd->id();
      }
      _side_heat_node_ids[_names[j]].push_back(nd->id());
    }
    _inner_heat_node_ids.push_back(node_ids[i][0]);
    _outer_heat_node_ids.push_back(node_ids[i][_total_elem_number * 2]);
  }

  // create elements from nodes
  unsigned int bc_id1 = getNextBoundaryId();
  unsigned int bc_id2 = getNextBoundaryId();
  _inner_bc_id.push_back(bc_id1);
  _outer_bc_id.push_back(bc_id2);

  _elem_ids.resize(_n_elem);
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    unsigned int n = 0;
    for (unsigned int j = 0; j < _number_of_hs; j++) // loop on all heat structures to add elements
    {
      for (unsigned int k = 0; k < _n_part_elems[j]; k++)
      {
        Elem * elem = _mesh.getMesh().add_elem(new Quad9);
        elem->subdomain_id() = _subdomain_ids[j];
        // vertices
        elem->set_node(0) = _mesh.getMesh().node_ptr(node_ids[2 * i][2 * n]);
        elem->set_node(1) = _mesh.getMesh().node_ptr(node_ids[2 * i][2 * (n + 1)]);
        elem->set_node(2) = _mesh.getMesh().node_ptr(node_ids[2 * (i + 1)][2 * (n + 1)]);
        elem->set_node(3) = _mesh.getMesh().node_ptr(node_ids[2 * (i + 1)][2 * n]);
        // mid-edges
        elem->set_node(4) = _mesh.getMesh().node_ptr(node_ids[2 * i][(2 * n) + 1]);
        elem->set_node(5) = _mesh.getMesh().node_ptr(node_ids[(2 * i) + 1][2 * (n + 1)]);
        elem->set_node(6) = _mesh.getMesh().node_ptr(node_ids[2 * (i + 1)][(2 * n) + 1]);
        elem->set_node(7) = _mesh.getMesh().node_ptr(node_ids[(2 * i) + 1][(2 * n)]);
        // center
        elem->set_node(8) = _mesh.getMesh().node_ptr(node_ids[(2 * i) + 1][(2 * n) + 1]);

        _elem_ids[i].push_back(elem->id());

        if (n == 0)
          _mesh.getMesh().boundary_info->add_side(elem, 3, bc_id1);
        if (n == _total_elem_number - 1)
          _mesh.getMesh().boundary_info->add_side(elem, 1, bc_id2);

        n++;
      }
    }
  }
}

void
HeatStructureBase::buildMesh()
{
  if (_n_part_elems.size() != _number_of_hs || _width.size() != _number_of_hs)
    return;

  // put heat structures in different blocks(sub domains)
  for (unsigned int i = 0; i < _number_of_hs; i++)
  {
    const std::string solid_block_name = genName(_name, _names[i]);
    unsigned int sid = getNextSubdomainId();
    // set the coordinate system for MOOSE, we do the RZ intergration ourselves until we can set
    // arbitrary number of axis symmetries in MOOSE
    setSubdomainInfo(sid, solid_block_name, Moose::COORD_XYZ);
  }

  if (usingSecondOrderMesh())
    build2DMesh2ndOrder();
  else
    build2DMesh();

  for (auto & bnd_id : _inner_bc_id)
  {
    const BoundaryName boundary_name = genName(name(), "inner");
    _mesh.setBoundaryName(bnd_id, boundary_name);
    _boundary_names_inner.push_back(boundary_name);
  }
  for (auto & bnd_id : _outer_bc_id)
  {
    const BoundaryName boundary_name = genName(name(), "outer");
    _mesh.setBoundaryName(bnd_id, boundary_name);
    _boundary_names_outer.push_back(boundary_name);
  }
}

void
HeatStructureBase::addVariables()
{
  _hc_model->addVariables();
  if (isParamValid("initial_T"))
    _hc_model->addInitialConditions();
}

void
HeatStructureBase::addMooseObjects()
{
  if (parameters().isParamValid("materials"))
  {
    _hc_model->addMaterials();

    for (unsigned int i = 0; i < _number_of_hs; i++)
    {
      const SolidMaterialProperties & smp =
          _sim.getUserObject<SolidMaterialProperties>(_material_names[i]);

      Component * comp = (_parent != nullptr) ? _parent : this;
      // if the values were given as constant, allow them to be controlled
      const ConstantFunction * k_fn = dynamic_cast<const ConstantFunction *>(&smp.getKFunction());
      if (k_fn != nullptr)
        comp->connectObject(k_fn->parameters(), k_fn->name(), "k", "value");

      const ConstantFunction * cp_fn = dynamic_cast<const ConstantFunction *>(&smp.getCpFunction());
      if (cp_fn != nullptr)
        comp->connectObject(cp_fn->parameters(), cp_fn->name(), "cp", "value");

      const ConstantFunction * rho_fn =
          dynamic_cast<const ConstantFunction *>(&smp.getRhoFunction());
      if (rho_fn != nullptr)
        comp->connectObject(rho_fn->parameters(), rho_fn->name(), "rho", "value");
    }
  }
}

FunctionName
HeatStructureBase::getInitialT() const
{
  if (isParamValid("initial_T"))
    return getParam<FunctionName>("initial_T");
  else
    mooseError(name(), ": The parameter 'initial_T' was requested but not supplied");
}

const std::vector<unsigned int> &
HeatStructureBase::getSideNodeIds(const std::string & name) const
{
  checkSetupStatus(MESH_PREPARED);

  return _side_heat_node_ids.at(name);
}

const std::vector<unsigned int> &
HeatStructureBase::getOuterNodeIds() const
{
  checkSetupStatus(MESH_PREPARED);

  return _outer_heat_node_ids;
}

const std::vector<BoundaryName> &
HeatStructureBase::getOuterBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_outer;
}

const std::vector<BoundaryName> &
HeatStructureBase::getInnerBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_inner;
}
