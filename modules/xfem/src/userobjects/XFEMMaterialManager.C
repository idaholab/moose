/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMMaterialManager.h"

#include "MooseMesh.h"
#include "Material.h"

#include "libmesh/mesh_base.h"

template <>
InputParameters
validParams<XFEMMaterialManager>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Manage the execution of stateful materials on extra QPs");
  params.addRequiredParam<std::vector<std::string>>("material_names",
                                                    "List of recompute material objects manage");
  params.set<MultiMooseEnum>("execute_on") = "initial linear";
  return params;
}

XFEMMaterialManager::XFEMMaterialManager(const InputParameters & parameters)
  : GeneralUserObject(parameters), _mesh(_fe_problem.mesh().getMesh())
{
  // get MaterialData entries for all listed material properties
  for (auto name : getParam<std::vector<std::string>>("material_names"))
  {
    auto & material = getMaterial(name);

    // get the material properties
    auto prop_list = material.getSuppliedItems();
    for (auto & prop : prop_list)
    {
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

XFEMMaterialManager::~XFEMMaterialManager()
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
XFEMMaterialManager::rewind()
{
  // got back a time step (note: use of older is unreliable)
  _map.swap(_map_older);
  _map.swap(_map_old);
}

void
XFEMMaterialManager::initialize()
{
}

void
XFEMMaterialManager::execute()
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
      item_older.push_back(_props_old[item_older.size()]->init(n_extra_qps));

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
    _fe_problem.reinitElemPhys(_mesh.elem_ptr(extra_qps.first), extra_qps.second, 0 /* tid */);

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
XFEMMaterialManager::finalize()
{
}
