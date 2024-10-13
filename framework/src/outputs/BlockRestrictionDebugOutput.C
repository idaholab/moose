//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "KernelBase.h"
#include "AuxiliarySystem.h"
#include "AuxKernel.h"

#include "libmesh/transient_system.h"

using namespace libMesh;

registerMooseObject("MooseApp", BlockRestrictionDebugOutput);

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

  params.addClassDescription(
      "Debug output object for displaying information regarding block-restriction of objects.");
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
    _sys(_nl.system())
{
}

void
BlockRestrictionDebugOutput::output()
{
  printBlockRestrictionMap();
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
