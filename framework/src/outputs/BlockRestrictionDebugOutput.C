//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BlockRestrictionDebugOutput.h"
#include "FEProblem.h"
#include "Material.h"
#include "ConsoleUtils.h"
#include "MooseMesh.h"
#include "MooseObjectName.h"
#include "NonlinearSystemBase.h"
#include "MooseVariableBase.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "KernelBase.h"
#include "AuxiliarySystem.h"
#include "AuxKernel.h"
#include "UserObject.h"
#include "MooseObject.h"
#include "InitialConditionBase.h"
#include "FVInitialConditionBase.h"
#include "Constraint.h"

#include "libmesh/transient_system.h"

#include <functional>
#include <map>

using namespace libMesh;

registerMooseObject("MooseApp", BlockRestrictionDebugOutput);

namespace
{
template <typename ID>
using RestrictionGroups = std::map<std::set<ID>, std::set<std::string>>;

template <typename ID>
std::string
formatRestrictionIDs(const std::set<ID> & ids,
                     const std::set<ID> & all_ids,
                     const std::string & all_text,
                     const std::function<std::string(ID)> & id_to_string)
{
  if (ids.empty())
    return "(none)";

  if (ids == all_ids)
    return all_text;

  std::stringstream out;
  unsigned int i = 0;
  for (const auto id : ids)
    out << id_to_string(id) << (++i < ids.size() ? ", " : "");

  return out.str();
}

void
printGroupNames(std::stringstream & out, const std::set<std::string> & names)
{
  std::streampos begin_string_pos = out.tellp();
  std::streampos curr_string_pos = begin_string_pos;
  unsigned int i = 0;
  for (const auto & name : names)
  {
    out << Moose::stringify(name) << (++i < names.size() ? ", " : "");
    curr_string_pos = out.tellp();
    ConsoleUtils::insertNewline(out, begin_string_pos, curr_string_pos);
  }
  out << '\n';
}

std::string
objectRestrictionName(const MooseObject & object)
{
  return object.type() + "/" + object.name();
}

void
addBlockRestrictionObject(RestrictionGroups<SubdomainID> & groups, const MooseObject & object)
{
  if (!object.enabled())
    return;

  const auto * const block_restrictable = dynamic_cast<const BlockRestrictable *>(&object);
  if (block_restrictable)
    groups[block_restrictable->blockIDs()].insert(objectRestrictionName(object));
}

void
addBoundaryRestrictionObject(RestrictionGroups<BoundaryID> & groups,
                             const MooseObject & object,
                             const bool include_unrestricted)
{
  if (!object.enabled())
    return;

  const auto * const boundary_restrictable = dynamic_cast<const BoundaryRestrictable *>(&object);
  if (!boundary_restrictable)
    return;

  if (!include_unrestricted && !boundary_restrictable->boundaryRestricted())
    return;

  const auto & ids = boundary_restrictable->boundaryRestricted()
                         ? boundary_restrictable->boundaryIDs()
                         : boundary_restrictable->meshBoundaryIDs();
  groups[ids].insert(objectRestrictionName(object));
}

template <typename T>
void
addWarehouseBlockRestrictionObjects(RestrictionGroups<SubdomainID> & groups,
                                    const MooseObjectWarehouseBase<T> & warehouse)
{
  for (const auto & object : warehouse.getObjects(/*tid = */ 0))
    addBlockRestrictionObject(groups, *object);
}

template <typename T>
void
addWarehouseBoundaryRestrictionObjects(RestrictionGroups<BoundaryID> & groups,
                                       const MooseObjectWarehouseBase<T> & warehouse,
                                       const bool include_unrestricted)
{
  for (const auto & object : warehouse.getObjects(/*tid = */ 0))
    addBoundaryRestrictionObject(groups, *object, include_unrestricted);
}
}

InputParameters
BlockRestrictionDebugOutput::validParams()
{
  InputParameters params = Output::validParams();

  params.addParam<MultiMooseEnum>("scope",
                                  getScopes("all"),
                                  "The types of object to output block coverage for, if nothing is "
                                  "provided everything will be output.");

  params.addParam<NonlinearSystemName>(
      "nl_sys", "nl0", "The nonlinear system that we should output information for.");
  params.addParam<bool>(
      "show_block_restriction_map",
      true,
      "Print active objects for each block. This is the default block-restriction debug output.");
  params.addParam<bool>("show_block_restriction_groups",
                        false,
                        "Print groups of objects with identical block restrictions.");
  params.addParam<bool>("show_boundary_restriction_groups",
                        false,
                        "Print groups of objects with identical boundary restrictions.");

  params.addClassDescription(
      "Debug output object for displaying information regarding block and boundary restrictions of "
      "objects.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

MultiMooseEnum
BlockRestrictionDebugOutput::getScopes(std::string default_scopes)
{
  return MultiMooseEnum("none all variables kernels auxvariables auxkernels materials userobjects",
                        default_scopes);
}

BlockRestrictionDebugOutput::BlockRestrictionDebugOutput(const InputParameters & parameters)
  : Output(parameters),
    _scope(getParam<MultiMooseEnum>("scope")),
    _nl(_problem_ptr->getNonlinearSystemBase(
        _problem_ptr->nlSysNum(getParam<NonlinearSystemName>("nl_sys")))),
    _sys(_nl.system()),
    _show_block_restriction_map(getParam<bool>("show_block_restriction_map")),
    _show_block_restriction_groups(getParam<bool>("show_block_restriction_groups")),
    _show_boundary_restriction_groups(getParam<bool>("show_boundary_restriction_groups"))
{
}

void
BlockRestrictionDebugOutput::output()
{
  if (_show_block_restriction_map)
    printBlockRestrictionMap();

  if (_show_block_restriction_groups)
    printBlockRestrictionGroups();

  if (_show_boundary_restriction_groups)
    printBoundaryRestrictionGroups();
}

void
BlockRestrictionDebugOutput::printBlockRestrictionMap() const
{
  // is there anything to do?
  if (_scope.isValid() && _scope.contains("none"))
    return;

  // Build output stream
  std::stringstream out;

  auto printCategoryAndNames = [&out](std::string category, std::set<std::string> & names)
  {
    const auto n = names.size();
    if (n > 0)
    {
      // print the category and number of items
      out << "    " << category << " (" << n << " " << ((n == 1) ? "item" : "items") << "): ";

      // If we would just use Moose::stringify(names), the indention is not right.
      // So we are printing the names one by one and using ConsoleUtils::insertNewline.
      std::streampos begin_string_pos = out.tellp();
      std::streampos curr_string_pos = begin_string_pos;
      unsigned int i = 0;
      for (const auto & name : names)
      {
        out << Moose::stringify(name) << ((i++ < (n - 1)) ? ", " : "");
        curr_string_pos = out.tellp();
        ConsoleUtils::insertNewline(out, begin_string_pos, curr_string_pos);
      }
      out << '\n';
      return true;
    }
    else
    {
      return false;
    }
  };

  // Reference to mesh for getting block names
  MooseMesh & mesh = _problem_ptr->mesh();

  // set of all subdomains in the mesh
  const std::set<SubdomainID> & mesh_subdomains = mesh.meshSubdomains();

  // get kernels via reference to the Kernel warehouse
  const auto & kernel_warehouse = _nl.getKernelWarehouse();
  const auto & kernels = kernel_warehouse.getObjects(/*tid = */ 0);

  // AuxSystem
  const auto & auxSystem = _problem_ptr->getAuxiliarySystem();

  // Reference to the Material warehouse
  const auto & material_warehouse = _problem_ptr->getMaterialWarehouse();

  // get all user objects
  std::vector<UserObject *> userObjects;
  _problem_ptr->theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribThread>(0)
      .queryIntoUnsorted(userObjects);

  // do we have to check all object types?
  const bool include_all = !_scope.isValid() || _scope.contains("all");

  // iterate all subdomains
  for (const auto & subdomain_id : mesh_subdomains)
  {
    // get the corresponding subdomain name
    const auto & subdomain_name = mesh.getSubdomainName(subdomain_id);

    out << "\n";
    out << "  Subdomain '" << subdomain_name << "' (id " << subdomain_id << "):\n";

    bool objectsFound = false;

    // Variables
    if (include_all || _scope.contains("variables"))
    {
      std::set<std::string> names;
      for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
      {
        const auto & var_name = _sys.variable_name(var_num);
        if (_problem_ptr->hasVariable(var_name))
        {
          const MooseVariableBase & var =
              _problem_ptr->getVariable(/*tid = */ 0,
                                        var_name,
                                        Moose::VarKindType::VAR_ANY,
                                        Moose::VarFieldType::VAR_FIELD_ANY);
          if (var.hasBlocks(subdomain_id))
            names.insert(var_name);
        }
      }
      objectsFound = printCategoryAndNames("Variables", names) || objectsFound;
    }

    // Kernels
    if (include_all || _scope.contains("kernels"))
    {
      std::set<std::string> names;
      for (const auto & kernel : kernels)
      {
        if (kernel->hasBlocks(subdomain_id))
          names.insert(kernel->name());
      }
      objectsFound = printCategoryAndNames("Kernels", names) || objectsFound;
    }

    // AuxVariables
    if (include_all || _scope.contains("auxvariables"))
    {
      std::set<std::string> names;
      const auto & sys = auxSystem.system();
      for (unsigned int vg = 0; vg < sys.n_variable_groups(); vg++)
      {
        const VariableGroup & vg_description(sys.variable_group(vg));
        for (unsigned int vn = 0; vn < vg_description.n_variables(); vn++)
        {
          if (vg_description.active_on_subdomain(subdomain_id))
            names.insert(vg_description.name(vn));
        }
      }
      objectsFound = printCategoryAndNames("AuxVariables", names) || objectsFound;
    }

    // AuxKernels
    if (include_all || _scope.contains("auxkernels"))
    {

      {
        const auto & wh = auxSystem.nodalAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[nodal]", names) || objectsFound;
      }

      {
        const auto & wh = auxSystem.nodalVectorAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[nodalVector]", names) || objectsFound;
      }

      {
        const auto & wh = auxSystem.nodalArrayAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[nodalArray]", names) || objectsFound;
      }

      {
        const auto & wh = auxSystem.elemAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[elemAux]", names) || objectsFound;
      }

      {
        const auto & wh = auxSystem.elemVectorAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[elemVector]", names) || objectsFound;
      }

      {
        const auto & wh = auxSystem.elemArrayAuxWarehouse();
        std::set<std::string> names;
        if (wh.hasActiveBlockObjects(subdomain_id))
        {
          const auto & auxkernels = wh.getActiveBlockObjects(subdomain_id);
          for (auto & auxkernel : auxkernels)
            names.insert(auxkernel->name());
        }
        objectsFound = printCategoryAndNames("AuxKernels[elemArray]", names) || objectsFound;
      }
    }

    // Materials
    if (include_all || _scope.contains("materials"))
    {
      std::set<std::string> names;
      if (material_warehouse.hasActiveBlockObjects(subdomain_id))
      {
        auto const objs = material_warehouse.getBlockObjects(subdomain_id);
        for (const auto & mat : objs)
          names.insert(mat->name());
      }
      objectsFound = printCategoryAndNames("Materials", names) || objectsFound;
    }

    // UserObjects
    if (include_all || _scope.contains("userobjects"))
    {
      std::set<std::string> names;
      for (const auto & obj : userObjects)
        if (BlockRestrictable * blockrestrictable_obj = dynamic_cast<BlockRestrictable *>(obj))
          if (blockrestrictable_obj->hasBlocks(subdomain_id))
            names.insert(obj->name());
      objectsFound = printCategoryAndNames("UserObjects", names) || objectsFound;
    }

    if (!objectsFound)
      out << "    (no objects found)\n";
  }

  out << std::flush;

  // Write the stored string to the ConsoleUtils output objects
  _console << "\n[DBG] Block-Restrictions (" << mesh_subdomains.size()
           << " subdomains): showing active objects\n";
  _console << std::setw(ConsoleUtils::console_field_width) << out.str() << std::endl;
}

void
BlockRestrictionDebugOutput::printBlockRestrictionGroups() const
{
  MooseMesh & mesh = _problem_ptr->mesh();
  const auto & mesh_subdomains = mesh.meshSubdomains();

  RestrictionGroups<SubdomainID> groups;
  std::vector<MooseObject *> objects;
  _problem_ptr->theWarehouse()
      .query()
      .condition<AttribInterfaces>(Interfaces::BlockRestrictable)
      .condition<AttribThread>(0)
      .queryIntoUnsorted(objects);

  for (const auto object : objects)
    if (object->enabled())
    {
      const auto * const block_restrictable = dynamic_cast<const BlockRestrictable *>(object);
      mooseAssert(block_restrictable, "Query returned an object without BlockRestrictable");
      groups[block_restrictable->blockIDs()].insert(objectRestrictionName(*object));
    }

  // Variables and aux variables are stored in libMesh systems / VariableWarehouse, not in
  // theWarehouse(), so add their block restrictions explicitly.
  for (const auto var_num : make_range(_sys.n_vars()))
  {
    const auto & var_name = _sys.variable_name(var_num);
    if (_problem_ptr->hasVariable(var_name))
    {
      const auto & var = _problem_ptr->getVariable(
          /*tid = */ 0, var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
      groups[var.blockIDs()].insert("Variable/" + var_name);
    }
  }

  const auto & aux_system = _problem_ptr->getAuxiliarySystem().system();
  for (const auto vg : make_range(aux_system.n_variable_groups()))
  {
    const VariableGroup & vg_description(aux_system.variable_group(vg));
    std::set<SubdomainID> blocks;
    for (const auto subdomain_id : mesh_subdomains)
      if (vg_description.active_on_subdomain(subdomain_id))
        blocks.insert(subdomain_id);

    for (const auto vn : make_range(vg_description.n_variables()))
      groups[blocks].insert("AuxVariable/" + vg_description.name(vn));
  }

  // Custom warehouses below are not covered by theWarehouse() queries.
  const auto & aux_system_base = _problem_ptr->getAuxiliarySystem();
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.nodalAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.mortarNodalAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.nodalVectorAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.nodalArrayAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.elemAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.elemVectorAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.elemArrayAuxWarehouse());
#ifdef MOOSE_KOKKOS_ENABLED
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.kokkosNodalAuxWarehouse());
  addWarehouseBlockRestrictionObjects(groups, aux_system_base.kokkosElemAuxWarehouse());
#endif

  addWarehouseBlockRestrictionObjects(groups, _problem_ptr->getInitialConditionWarehouse());
  addWarehouseBlockRestrictionObjects(groups, _problem_ptr->getFVInitialConditionWarehouse());
  addWarehouseBlockRestrictionObjects(groups, _nl.getConstraintWarehouse());

  // Materials use MaterialWarehouse, which also owns automatically-created face and neighbor
  // materials; report the primary material objects from the aggregate material warehouse.
  addWarehouseBlockRestrictionObjects(groups, _problem_ptr->getMaterialWarehouse());

  std::stringstream out;
  for (const auto & group : groups)
  {
    out << "\n";
    out << "  Blocks "
        << formatRestrictionIDs<SubdomainID>(group.first,
                                             mesh_subdomains,
                                             "all blocks",
                                             [&mesh](const SubdomainID id)
                                             {
                                               const auto & name = mesh.getSubdomainName(id);
                                               return name.empty() ? std::to_string(id)
                                                                   : "'" + name + "' (id " +
                                                                         std::to_string(id) + ")";
                                             })
        << " (" << group.second.size() << " " << (group.second.size() == 1 ? "item" : "items")
        << "): ";
    printGroupNames(out, group.second);
  }

  if (groups.empty())
    out << "\n  (no objects found)\n";

  out << std::flush;

  _console << "\n[DBG] Block-Restriction Groups (" << groups.size()
           << " groups): showing objects with matching block restrictions\n";
  _console << std::setw(ConsoleUtils::console_field_width) << out.str() << std::endl;
}

void
BlockRestrictionDebugOutput::printBoundaryRestrictionGroups() const
{
  MooseMesh & mesh = _problem_ptr->mesh();
  const auto & mesh_boundaries = mesh.getBoundaryIDs();

  RestrictionGroups<BoundaryID> groups;
  std::vector<MooseObject *> objects;
  _problem_ptr->theWarehouse()
      .query()
      .condition<AttribInterfaces>(Interfaces::BoundaryRestrictable)
      .condition<AttribThread>(0)
      .queryIntoUnsorted(objects);

  for (const auto object : objects)
    if (object->enabled())
    {
      const auto * const boundary_restrictable = dynamic_cast<const BoundaryRestrictable *>(object);
      mooseAssert(boundary_restrictable, "Query returned an object without BoundaryRestrictable");
      const auto & ids = boundary_restrictable->boundaryRestricted()
                             ? boundary_restrictable->boundaryIDs()
                             : boundary_restrictable->meshBoundaryIDs();
      groups[ids].insert(objectRestrictionName(*object));
    }

  // Custom warehouses below are not covered by theWarehouse() queries. For these explicit passes,
  // only boundary-restricted objects belong in boundary-restriction groups; block-only objects are
  // already represented in the block groups.
  const auto & aux_system = _problem_ptr->getAuxiliarySystem();
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.nodalAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.mortarNodalAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.nodalVectorAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.nodalArrayAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.elemAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.elemVectorAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.elemArrayAuxWarehouse(), false);
#ifdef MOOSE_KOKKOS_ENABLED
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.kokkosNodalAuxWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, aux_system.kokkosElemAuxWarehouse(), false);
#endif

  addWarehouseBoundaryRestrictionObjects(groups, _problem_ptr->getInitialConditionWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, _nl.getConstraintWarehouse(), false);
  addWarehouseBoundaryRestrictionObjects(groups, _problem_ptr->getMaterialWarehouse(), false);

  std::stringstream out;
  for (const auto & group : groups)
  {
    out << "\n";
    out << "  Boundaries "
        << formatRestrictionIDs<BoundaryID>(group.first,
                                            mesh_boundaries,
                                            "all boundaries",
                                            [&mesh](const BoundaryID id)
                                            {
                                              const auto name = mesh.getBoundaryString(id);
                                              return "'" + name + "' (id " + std::to_string(id) +
                                                     ")";
                                            })
        << " (" << group.second.size() << " " << (group.second.size() == 1 ? "item" : "items")
        << "): ";
    printGroupNames(out, group.second);
  }

  if (groups.empty())
    out << "\n  (no objects found)\n";

  out << std::flush;

  _console << "\n[DBG] Boundary-Restriction Groups (" << groups.size()
           << " groups): showing objects with matching boundary restrictions\n";
  _console << std::setw(ConsoleUtils::console_field_width) << out.str() << std::endl;
}
