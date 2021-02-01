/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMElemPairMaterialManager.h"
#include "XFEM.h"
#include "MooseMesh.h"
#include "Material.h"
#include "ElementPairQPProvider.h"

#include "libmesh/mesh_base.h"

registerMooseObject("XFEMApp", XFEMElemPairMaterialManager);

InputParameters
XFEMElemPairMaterialManager::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Manage the execution of stateful materials on extra QPs");
  params.addRequiredParam<std::vector<std::string>>("material_names",
                                                    "List of recompute material objects manage");
  params.addRequiredParam<UserObjectName>("element_pair_qps",
                                          "Object that provides the extra QPs for element pair.");
  params.set<MultiMooseEnum>("execute_on") = "initial linear";
  return params;
}

XFEMElemPairMaterialManager::XFEMElemPairMaterialManager(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh().getMesh()),
    _extra_qp_map(getUserObject<ElementPairQPProvider>("element_pair_qps").getExtraQPMap()),
    _elem_pair_map(getUserObject<ElementPairQPProvider>("element_pair_qps").getElementPairMap())
{
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  _elem_pair_unique_id_map = &(_xfem->getElemPairUniqueIDMap());
}

void
XFEMElemPairMaterialManager::initialSetup()
{
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
  for (auto & item : *_map)
    item.second.destroy();
  for (auto & item : *_map_old)
    item.second.destroy();
  for (auto & item : *_map_older)
    item.second.destroy();
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
  std::cout << "============> BEFORE id = ";
  for (auto key_id : *_map)
  {
    std::cout << key_id.first << ", ";
  }
  std::cout << std::endl;

  for (std::map<unique_id_type, unique_id_type>::iterator it = (*_elem_pair_unique_id_map).begin();
       it != (*_elem_pair_unique_id_map).end();
       ++it)
  {
    std::cout << "XFEMElemPairMaterialManager --> old unique id = " << it->first
              << ", new unique id = " << it->second << std::endl;

    auto it_delete_map = (*_map).find(it->first);
    if (it_delete_map != (*_map).end())
    {
      std::cout << "MAP find item : " << it->first << std::endl;
      std::swap((*_map)[it->second], it_delete_map->second);
      (*_map).erase(it_delete_map);
    }

    auto it_delete_map_old = (*_map_old).find(it->first);
    if (it_delete_map_old != (*_map_old).end())
    {
      std::swap((*_map_old)[it->second], it_delete_map_old->second);
      (*_map_old).erase(it_delete_map_old);
    }

    auto it_delete_map_older = (*_map_older).find(it->first);
    if (it_delete_map_older != (*_map_older).end())
    {
      std::swap((*_map_older)[it->second], it_delete_map_older->second);
      (*_map_older).erase(it_delete_map_older);
    }
  }
  std::cout << "============> AFTER id = ";
  for (auto key_id : *_map)
  {
    std::cout << key_id.first << ", ";
  }
  std::cout << std::endl;
}

void
XFEMElemPairMaterialManager::execute()
{
  // fetch all variable dependencies
  std::set<MooseVariableFieldBase *> var_dependencies;
  for (auto & material : _materials)
  {
    auto & material_var_dependencies = material->getMooseVariableDependencies();
    var_dependencies.insert(material_var_dependencies.begin(), material_var_dependencies.end());
  }

  _fe_problem.setActiveElementalMooseVariables(var_dependencies, 0);

  std::cout << "XFEMElemPairMaterialManager map size = " << _map->size() << std::endl;
  std::cout << "_extra_qp_map size = " << _extra_qp_map.size() << std::endl;

  std::cout << "id = ";
  for (auto key_id : *_map)
  {
    std::cout << key_id.first << ", ";
  }
  std::cout << std::endl;

  // loop over all elements that have extra QPs
  for (auto extra_qps : _extra_qp_map)
  {
    // number of extra QPs in this timestep
    const auto n_extra_qps = extra_qps.second.size();

    // fetch property history for this element
    auto & item = (*_map)[extra_qps.first];
    auto & item_old = (*_map_old)[extra_qps.first];
    auto & item_older = (*_map_older)[extra_qps.first];

    unsigned int n_old_extra_qps = 0;
    if (item.size() > 0)
    {
      n_old_extra_qps = item[0]->size();
      mooseAssert(n_old_extra_qps == item_old[0]->size(), "Inconsistent history item sizes");
      mooseAssert(n_old_extra_qps == item_older[0]->size(), "Inconsistent history item sizes");
    }

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

    const Elem * elem1 = _elem_pair_map.at(extra_qps.first).first;
    const Elem * elem2 = _elem_pair_map.at(extra_qps.first).second;

    std::cout << "elem1 unique id = " << elem1->unique_id()
              << ", elem 2 unique id = " << elem2->unique_id() << std::endl;

    // std::cout << "elem 1 =========================================== " << *elem1 << std::endl;
    // std::cout << "elem 2 =========================================== " << *elem2 << std::endl;

    // reinit the element
    _fe_problem.setCurrentSubdomainID(elem1, 0 /* tid */);
    _fe_problem.reinitElemPhys(elem1, extra_qps.second, 0 /* tid */);
    // reinit the neighbor element
    _fe_problem.setNeighborSubdomainID(elem2, 0 /* tid */);
    _fe_problem.reinitNeighborPhys(elem2, extra_qps.second, 0 /* tid */);

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
XFEMElemPairMaterialManager::swapInProperties(unique_id_type elem_id)
{
  auto & item = (*_map)[elem_id];
  auto & item_old = (*_map_old)[elem_id];
  auto & item_older = (*_map_older)[elem_id];

  // swap the history in for all properties
  for (auto i = beginIndex(_props); i < _props.size(); ++i)
    _props[i]->swap(item[i]);
  for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
    _props_old[i]->swap(item_old[i]);
  for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
    _props_older[i]->swap(item_older[i]);
}

void
XFEMElemPairMaterialManager::swapOutProperties(unique_id_type elem_id)
{
  auto & item = (*_map)[elem_id];
  auto & item_old = (*_map_old)[elem_id];
  auto & item_older = (*_map_older)[elem_id];

  // swap the history in for all properties
  for (auto i = beginIndex(_props); i < _props.size(); ++i)
    _props[i]->swap(item[i]);
  for (auto i = beginIndex(_props_old); i < _props_old.size(); ++i)
    _props_old[i]->swap(item_old[i]);
  for (auto i = beginIndex(_props_older); i < _props_older.size(); ++i)
    _props_older[i]->swap(item_older[i]);
}

void
XFEMElemPairMaterialManager::swapInProperties(unique_id_type elem_id) const
{
  const_cast<XFEMElemPairMaterialManager *>(this)->swapInProperties(elem_id);
}

void
XFEMElemPairMaterialManager::swapOutProperties(unique_id_type elem_id) const
{
  const_cast<XFEMElemPairMaterialManager *>(this)->swapOutProperties(elem_id);
}

unsigned int
XFEMElemPairMaterialManager::materialPropertyIndex(const std::string & name) const
{
  auto it = _managed_properties.find(name);
  if (it == _managed_properties.end())
    mooseError("Property '", name, "' is not managed by this object");

  return it->second;
}
