/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "MaterialPropertyDebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"
#include "ConsoleUtils.h"

// libMesh includesx
#include "libmesh/transient_system.h"

template<>
InputParameters validParams<MaterialPropertyDebugOutput>()
{
  InputParameters params = validParams<BasicOutput<Output> >();

  // This object only outputs data once, in the constructor, so disable fine control
  params.suppressParameter<MultiMooseEnum>("output_on");

  return params;
}

MaterialPropertyDebugOutput::MaterialPropertyDebugOutput(const std::string & name, InputParameters & parameters) :
    BasicOutput<Output>(name, parameters)
{
  printMaterialMap();
}

MaterialPropertyDebugOutput::~MaterialPropertyDebugOutput()
{
}

void
MaterialPropertyDebugOutput::output(const ExecFlagType & /*type*/)
{
}

void
MaterialPropertyDebugOutput::printMaterialMap() const
{

  // Get a reference to the material warehouse
  MaterialWarehouse & material_warehouse = _problem_ptr->getMaterialWarehouse(0);

  // Build output streams for block materials and block face materials
  std::stringstream active_block, active_face, active_neighbor;
  std::set<SubdomainID> blocks = material_warehouse.blocks();
  for (std::set<SubdomainID>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
  {
    // Active materials on blocks
    active_block << "    Block ID " << *it << ":\n";
    printMaterialProperties(active_block, material_warehouse.getMaterials(*it));

    // Active face materials on blocks
    active_face << "    Block ID " << *it << ":\n";
    printMaterialProperties(active_face, material_warehouse.getFaceMaterials(*it));

    // Active face materials on blocks
    active_neighbor << "    Block ID " << *it << ":\n";
    printMaterialProperties(active_neighbor, material_warehouse.getNeighborMaterials(*it));
  }

  // Build output stream for side set materials
  std::stringstream active_boundary;
  std::set<BoundaryID> boundaries = material_warehouse.boundaries();
  for (std::set<BoundaryID>::const_iterator it = boundaries.begin(); it != boundaries.end(); ++it)
  {
    // Active materials on blocks
    active_boundary << "    Boundary ID " << *it << ":\n";
    printMaterialProperties(active_boundary, material_warehouse.getBoundaryMaterials(*it));
  }

  // Write the stored strings to the ConsoleUtils output objects
  _console << "Materials:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << "  Active Materials on Subdomain:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_block.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "  Active Face Materials on Subdomain:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_face.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "  Active Neighboring Materials on Subdomain:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_neighbor.str() << '\n';

  _console << std::setw(ConsoleUtils::console_field_width) << "  Active Materials on Boundaries:\n";
  _console << std::setw(ConsoleUtils::console_field_width) << active_boundary.str() << '\n';
}

void
MaterialPropertyDebugOutput::printMaterialProperties(std::stringstream & output, const std::vector<Material * > & materials) const
{
  // Loop through all material objects
  for (std::vector<Material *>::const_iterator jt = materials.begin(); jt != materials.end(); ++jt)
  {
    // Get a list of properties for the current material
    const std::set<std::string> & props = (*jt)->getSuppliedItems();

    // Adds the material name to the output stream
    output << std::left << std::setw(ConsoleUtils::console_field_width) << "      Material Name: " << (*jt)->name() << '\n';

    // Build stream for properties using the ConsoleUtils helper functions to wrap the names if there are too many for one line
    std::streampos begin_string_pos = output.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    output << std::left << std::setw(ConsoleUtils::console_field_width) << "      Property Names: ";
    for (std::set<std::string>::const_iterator prop_it = props.begin(); prop_it != props.end(); ++prop_it)
    {
      output << "\"" << (*prop_it) << "\" ";
      curr_string_pos = output.tellp();
      ConsoleUtils::insertNewline(output, begin_string_pos, curr_string_pos);
    }
    output << '\n';
  }
}
