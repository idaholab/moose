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
#include "Console.h"

// libMesh includesx
#include "libmesh/transient_system.h"

template<>
InputParameters validParams<MaterialPropertyDebugOutput>()
{
  InputParameters params = validParams<PetscOutput>();

  // Suppress unnecessary parameters
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("output_vector_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");
  params.suppressParameter<bool>("elemental_as_nodal");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("output_input");
  params.suppressParameter<bool>("output_system_information");
  params.suppressParameter<bool>("file_base");

  // Create parameters for allowing debug outputter to be defined within the [Outputs] block
  params.addParam<bool>("show_var_residual_norms", false, "Print the residual norms of the individual solution variables at each nonlinear iteration");
  params.addParam<bool>("show_material_props", false, "Print out the material properties supplied for each block, face, neighbor, and/or sideset");
  params.addParam<unsigned int>("show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");

  // By default operate on linear residuals, this is to maintain the behavior of show_top_residuals
  params.set<bool>("linear_residuals") = true;

  return params;
}

MaterialPropertyDebugOutput::MaterialPropertyDebugOutput(const std::string & name, InputParameters & parameters) :
    Output(name, parameters)
{
  printMaterialMap();
}

MaterialPropertyDebugOutput::~MaterialPropertyDebugOutput()
{
}

void
MaterialPropertyDebugOutput::output()
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

  // Write the stored strings to the Console output objects
  _console << "Materials:\n";
  _console << std::setw(Console::_field_width) << "  Active Materials on Subdomain:\n";
  _console << std::setw(Console::_field_width) << active_block.str() << '\n';

  _console << std::setw(Console::_field_width) << "  Active Face Materials on Subdomain:\n";
  _console << std::setw(Console::_field_width) << active_face.str() << '\n';

  _console << std::setw(Console::_field_width) << "  Active Neighboring Materials on Subdomain:\n";
  _console << std::setw(Console::_field_width) << active_neighbor.str() << '\n';

  _console << std::setw(Console::_field_width) << "  Active Materials on Boundaries:\n";
  _console << std::setw(Console::_field_width) << active_boundary.str() << '\n';
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
    output << std::left << std::setw(Console::_field_width) << "      Material Name: " << (*jt)->name() << '\n';

    // Build stream for properties using the Console helper functions to wrap the names if there are too many for one line
    std::streampos begin_string_pos = output.tellp();
    std::streampos curr_string_pos = begin_string_pos;
    output << std::left << std::setw(Console::_field_width) << "      Property Names: ";
    for (std::set<std::string>::const_iterator prop_it = props.begin(); prop_it != props.end(); ++prop_it)
    {
      output << "\"" << (*prop_it) << "\" ";
      curr_string_pos = output.tellp();
      Console::insertNewline(output, begin_string_pos, curr_string_pos);
    }
    output << '\n';
  }
}

void
MaterialPropertyDebugOutput::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for Debug output");
}

void
MaterialPropertyDebugOutput::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for Debug output");
}

void
MaterialPropertyDebugOutput::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for Debug output");
}

void
MaterialPropertyDebugOutput::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not support for Debug output");
}

void
MaterialPropertyDebugOutput::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for Debug output");
}
