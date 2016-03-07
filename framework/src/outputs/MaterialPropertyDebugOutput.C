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
#include "MaterialBase.h"
#include "ConsoleUtils.h"

// libMesh includes
#include "libmesh/transient_system.h"

template<>
InputParameters validParams<MaterialPropertyDebugOutput>()
{
  InputParameters params = validParams<BasicOutput<Output> >();

  // This object only outputs data once, in the constructor, so disable fine control
  params.suppressParameter<MultiMooseEnum>("execute_on");

  return params;
}

MaterialPropertyDebugOutput::MaterialPropertyDebugOutput(const InputParameters & parameters) :
    BasicOutput<Output>(parameters)
{
  printMaterialMap();
}

void
MaterialPropertyDebugOutput::output(const ExecFlagType & /*type*/)
{
}

void
MaterialPropertyDebugOutput::printMaterialMap() const
{
  // Build output streams for block materials and block face materials
  std::stringstream active_block, active_face, active_neighbor, active_boundary;

  // Reference to the MaterialBase warehouse
  const MaterialWarehouse<MaterialBase> & warehouse = _problem_ptr->getMaterialWarehouse();


  // Active materials on block
  {
    const std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > > & objects = warehouse.getBlockObjects();
    for (std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
      active_block << "    Block ID " << it->first << ":\n";
      printMaterialProperties(active_block, it->second);
    }
  }

  // Active face materials on blocks
  {
    const std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > > & objects = warehouse[Moose::FACE_MATERIAL_DATA].getBlockObjects();
    for (std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
      active_face << "    Block ID " << it->first << ":\n";
      printMaterialProperties(active_face, it->second);
    }
  }

  // Active neighbor materials on blocks
  {
    const std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > > & objects = warehouse[Moose::NEIGHBOR_MATERIAL_DATA].getBlockObjects();
    for (std::map<SubdomainID, std::vector<MooseSharedPointer<MaterialBase> > >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
      active_neighbor << "    Block ID " << it->first << ":\n";
      printMaterialProperties(active_neighbor, it->second);
    }
  }

  // Active boundary materials
  {
    const std::map<BoundaryID, std::vector<MooseSharedPointer<MaterialBase> > > & objects = warehouse.getBoundaryObjects();
    for (std::map<BoundaryID, std::vector<MooseSharedPointer<MaterialBase> > >::const_iterator it = objects.begin(); it != objects.end(); ++it)
    {
      active_boundary << "    Boundary ID " << it->first << ":\n";
      printMaterialProperties(active_boundary, it->second);
    }
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
MaterialPropertyDebugOutput::printMaterialProperties(std::stringstream & output, const std::vector<MooseSharedPointer<MaterialBase> > & materials) const
{
  // Loop through all material objects
  for (std::vector<MooseSharedPointer<MaterialBase> >::const_iterator jt = materials.begin(); jt != materials.end(); ++jt)
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
