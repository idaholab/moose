/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMElemPairMaterialManager.h"

#include "MooseMesh.h"
#include "Material.h"

#include "libmesh/mesh_base.h"

template <>
InputParameters
validParams<XFEMElemPairMaterialManager>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Manage the execution of stateful materials on extra QPs");
  params.addRequiredParam<std::vector<std::string>>("material_names",
                                                    "List of recompute material objects manage");
  params.set<MultiMooseEnum>("execute_on") = "initial linear";
  return params;
}

XFEMElemPairMaterialManager::XFEMElemPairMaterialManager(const InputParameters & parameters)
  : GeneralUserObject(parameters), _mesh(_fe_problem.mesh().getMesh())
{
}

void
XFEMElemPairMaterialManager::initialSetup()
{
  std::cout << "XFEMElemPairMaterialManager::initialSetup()\n";

  // get MaterialData entries for all listed material properties
  for (auto name : getParam<std::vector<std::string>>("material_names"))
  {
    auto & material = getMaterialByName(name);

    // get the material properties
    auto prop_list = material.getSuppliedItems();
    for (auto & prop : prop_list)
    {
      // register managed property
      _managed_properties.insert(std::make_pair(prop, _props.size()));

      // get the property ID
      unsigned int prop_id = _material_data->getPropertyId(prop);

      // set up convenience links
      _props.push_back(_material_data->props()[prop_id]);
      _props_old.push_back(_material_data->propsOld()[prop_id]);
      _props_older.push_back(_material_data->propsOlder()[prop_id]);
    }

    // add to list of materials we need to execute (mind the order!)
    _materials.push_back(&material);
  }

  // allocate the history storage
  _map = std::unique_ptr<HistoryStorage>(new HistoryStorage);
  _map_old = std::unique_ptr<HistoryStorage>(new HistoryStorage);
  _map_older = std::unique_ptr<HistoryStorage>(new HistoryStorage);
}

void
XFEMElemPairMaterialManager::timestepSetup()
{
  // roll the history forward
  if (_fe_problem.converged())
  {
    _map.swap(_map_old);
    _map.swap(_map_older);
  }
}

XFEMElemPairMaterialManager::~XFEMElemPairMaterialManager()
{
  // destroy extra QP stateful property storage
  // for (auto & item : *_map)
  //   item.second.destroy();
  // for (auto & item : *_map_old)
  //   item.second.destroy();
  // for (auto & item : *_map_older)
  //   item.second.destroy();
}

void
XFEMElemPairMaterialManager::rewind()
{
  // got back a time step (note: use of older is unreliable)
  _map.swap(_map_older);
  _map.swap(_map_old);
}

void
XFEMElemPairMaterialManager::initialize()
{
  _extra_qp_map.clear();
  _elem_pair_id.clear();
  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * element_pair_locators = nullptr;

  GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
  element_pair_locators = &geom_search_data._element_pair_locators;

  for (const auto & it : *element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    // go over pair elements
    const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
        elem_pair_loc.getElemPairs();
    for (const auto & pr : elem_pairs)
    {
      const Elem * elem1 = pr.first;
      const Elem * elem2 = pr.second;

      if (elem1->processor_id() != processor_id())
        continue;

      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(pr);

      for (unsigned int i = 0; i < (info._elem1_constraint_q_point).size(); ++i)
        _extra_qp_map[elem1->id() + elem2->id()].push_back((info._elem1_constraint_q_point)[i]);

      _elem_pair_id[elem1->id() + elem2->id()] = std::make_pair(elem1->id(), elem2->id());
    }
  }
}

void
XFEMElemPairMaterialManager::execute()
{
  // fetch all variable dependencies
  std::set<MooseVariable *> var_dependencies;
  for (auto & material : _materials)
  {
    auto & material_var_dependencies = material->getMooseVariableDependencies();
    var_dependencies.insert(material_var_dependencies.begin(), material_var_dependencies.end());
  }
  _fe_problem.setActiveElementalMooseVariables(var_dependencies, 0);

  // loop over all elements that have extra QPs
  for (auto extra_qps : _extra_qp_map)
  {
    // number of extra QPs in this timestep
    const auto n_extra_qps = extra_qps.second.size();

    // fetch property history for this element
    auto & item = (*_map)[extra_qps.first];
    auto & item_old = (*_map_old)[extra_qps.first];
    auto & item_older = (*_map_older)[extra_qps.first];

    // number of extra QPs in the previous timestep (might have added QPs)
    const auto n_old_extra_qps = item.size();
    mooseAssert(n_old_extra_qps == item_old.size(), "Inconsistent history item sizes");
    mooseAssert(n_old_extra_qps == item_older.size(), "Inconsistent history item sizes");

    // make sure the items have room for the correct number of properties
    while (item.size() < _props.size())
      item.push_back(_props[item.size()]->init(n_extra_qps));
    while (item_old.size() < _props_old.size())
      item_old.push_back(_props_old[item_old.size()]->init(n_extra_qps));
    while (item_older.size() < _props_older.size())
      item_older.push_back(_props_older[item_older.size()]->init(n_extra_qps));

    // make sure it has the number of quadrature points
    item.resizeItems(n_extra_qps);
    item_old.resizeItems(n_extra_qps);
    item_older.resizeItems(n_extra_qps);

    // if we added new QPs we need to initialize the history by calling initQpStatefulProperties
    if (n_extra_qps > n_old_extra_qps)
    {
      for (auto & material : _materials)
        material->initStatefulProperties(n_extra_qps);

      // copy into history old position
      for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
        for (unsigned int qp = n_old_extra_qps; qp < n_extra_qps; ++qp)
          _props[i]->swap(item_old[i]);
    }

    // swap the history in for all properties
    for (auto i = beginIndex(_props); i < _props.size(); ++i)
      _props[i]->swap(item[i]);
    for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
      _props_old[i]->swap(item_old[i]);
    for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
      _props_older[i]->swap(item_older[i]);

    // reinit the element
    _fe_problem.reinitElemPhys(
        _mesh.elem_ptr(_elem_pair_id[extra_qps.first].first), extra_qps.second, 0 /* tid */);
    // reinit the neighbor element
    _fe_problem.reinitNeighborPhys(
        _mesh.elem_ptr(_elem_pair_id[extra_qps.first].second), extra_qps.second, 0 /* tid */);

    // loop over QPs
    for (unsigned int qp = 0; qp < extra_qps.second.size(); ++qp)
      // loop over materials (may have to handle exceptions to swap properties back!)
      for (auto & material : _materials)
        material->computePropertiesAtQp(qp);

    // swap the history in for all properties
    for (auto i = beginIndex(_props); i < _props.size(); ++i)
      _props[i]->swap(item[i]);
    for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
      _props_old[i]->swap(item_old[i]);
    for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
      _props_older[i]->swap(item_older[i]);
  }
}

void
XFEMElemPairMaterialManager::finalize()
{
}

void
XFEMElemPairMaterialManager::swapInProperties(dof_id_type pair_id)
{
  auto & item = (*_map)[pair_id];
  auto & item_old = (*_map_old)[pair_id];
  auto & item_older = (*_map_older)[pair_id];

  // swap the history in for all properties
  for (auto i = beginIndex(_props); i < _props.size(); ++i)
    _props[i]->swap(item[i]);
  for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
    _props_old[i]->swap(item_old[i]);
  for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
    _props_older[i]->swap(item_older[i]);
}

void
XFEMElemPairMaterialManager::swapOutProperties(dof_id_type pair_id)
{
  auto & item = (*_map)[pair_id];
  auto & item_old = (*_map_old)[pair_id];
  auto & item_older = (*_map_older)[pair_id];

  // swap the history in for all properties
  for (auto i = beginIndex(_props); i < _props.size(); ++i)
    _props[i]->swap(item[i]);
  for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
    _props_old[i]->swap(item_old[i]);
  for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
    _props_older[i]->swap(item_older[i]);
}

void
XFEMElemPairMaterialManager::swapInProperties(dof_id_type pair_id) const
{
  const_cast<XFEMElemPairMaterialManager *>(this)->swapInProperties(pair_id);
}

void
XFEMElemPairMaterialManager::swapOutProperties(dof_id_type pair_id) const
{
  const_cast<XFEMElemPairMaterialManager *>(this)->swapOutProperties(pair_id);
}

unsigned int
XFEMElemPairMaterialManager::materialPropertyIndex(const std::string & name) const
{
  auto it = _managed_properties.find(name);
  if (it == _managed_properties.end())
    mooseError("Property '", name, "' is not managed by this object");

  return it->second;
}
