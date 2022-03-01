//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialPropertyDebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"
#include "ConsoleUtils.h"
#include "MooseMesh.h"
#include "MooseObjectName.h"

#include "libmesh/transient_system.h"

registerMooseObject("MooseApp", MaterialPropertyDebugOutput);

InputParameters
MaterialPropertyDebugOutput::validParams()
{
  InputParameters params = Output::validParams();
  params.addClassDescription("Debug output object for displaying material property information.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

MaterialPropertyDebugOutput::MaterialPropertyDebugOutput(const InputParameters & parameters)
  : Output(parameters)
{
  printMaterialMap();
}

void
MaterialPropertyDebugOutput::output(const ExecFlagType & /*type*/)
{
  // Write out objects that consume properties
  std::stringstream consumed;
  for (const auto & pair : _problem_ptr->getConsumedPropertyMap())
  {
    consumed << "      Object: " << pair.first << "\n";
    consumed << "  Properties: " << MooseUtils::join(pair.second, ", ") << "\n\n";
  }

  _console << "\n\nConsumed Material Properties:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << consumed.str() << std::endl;
}

void
MaterialPropertyDebugOutput::printMaterialMap() const
{
  // Build output streams for block materials and block face materials
  std::stringstream active_block, active_face, active_neighbor, active_boundary;

  // Reference to mesh for getting boundary/block names
  MooseMesh & mesh = _problem_ptr->mesh();

  // Reference to the Material warehouse
  const MaterialWarehouse & warehouse = _problem_ptr->getMaterialWarehouse();

  // Active materials on block
  {
    const auto & objects = warehouse.getBlockObjects();
    for (const auto & it : objects)
    {
      active_block << "    Subdomain: " << mesh.getSubdomainName(it.first) << " (" << it.first
                   << ")\n";
      printMaterialProperties(active_block, it.second);
    }
  }

  // Active face materials on blocks
  {
    const auto & objects = warehouse[Moose::FACE_MATERIAL_DATA].getBlockObjects();
    for (const auto & it : objects)
    {
      active_face << "    Subdomain: " << mesh.getSubdomainName(it.first) << " (" << it.first
                  << ")\n";
      printMaterialProperties(active_face, it.second);
    }
  }

  // Active neighbor materials on blocks
  {
    const auto & objects = warehouse[Moose::NEIGHBOR_MATERIAL_DATA].getBlockObjects();
    for (const auto & it : objects)
    {
      active_neighbor << "    Subdomain: " << mesh.getSubdomainName(it.first) << " (" << it.first
                      << ")\n";
      printMaterialProperties(active_neighbor, it.second);
    }
  }

  // Active boundary materials
  {
    const auto & objects = warehouse.getBoundaryObjects();
    for (const auto & it : objects)
    {
      active_boundary << "    Boundary: " << mesh.getBoundaryName(it.first) << " (" << it.first
                      << ")\n";
      printMaterialProperties(active_boundary, it.second);
    }
  }

  // Write the stored strings to the ConsoleUtils output objects
  _console << "\n\nActive Materials:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_block.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "Active Face Materials:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_face.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "Active Neighboring Materials:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_neighbor.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "Active Boundary Materials:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_boundary.str() << std::endl;
}

void
MaterialPropertyDebugOutput::printMaterialProperties(
    std::stringstream & output, const std::vector<std::shared_ptr<MaterialBase>> & materials) const
{
  // Loop through all material objects
  for (const auto & mat : materials)
  {
    // Get a list of properties for the current material
    const std::set<std::string> & props = mat->getSuppliedItems();

    // Adds the material name to the output stream
    output << std::left << std::setw(ConsoleUtils::console_field_width)
           << "      Material Name: " << mat->name() << '\n';

    // Build stream for properties using the ConsoleUtils helper functions to wrap the names if
    // there are too many for one line
    std::streampos begin_string_pos = output.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    output << std::left << std::setw(ConsoleUtils::console_field_width) << "      Property Names: ";
    for (const auto & prop : props)
    {
      output << "\"" << prop << "\" ";
      curr_string_pos = output.tellp();
      ConsoleUtils::insertNewline(output, begin_string_pos, curr_string_pos);
    }
    output << '\n';
  }

  output << std::flush;
}
