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
#include "DebugOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Material.h"

// libMesh includesx
#include "libmesh/transient_system.h"

template<>
InputParameters validParams<DebugOutput>()
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

  return params;
}

DebugOutput::DebugOutput(const std::string & name, InputParameters & parameters) :
    PetscOutput(name, parameters),
    _show_var_residual_norms(getParam<bool>("show_var_residual_norms")),
    _sys(_problem_ptr->getNonlinearSystem().sys())
{
  // Force this outputter to output on nonlinear residuals
  _output_nonlinear = true;

  // Show material information
  if (getParam<bool>("show_material_props"))
    printMaterialMap();
}

DebugOutput::~DebugOutput()
{
}

void
DebugOutput::output()
{
  if (_show_var_residual_norms && onNonlinearResidual())
  {
    // Stream for outputting
    std::ostringstream oss;

    // Determine the maximum variable name size
    unsigned int max_name_size = 0;
    for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
    {
      unsigned int var_name_size = _sys.variable_name(var_num).size();
      if (var_name_size > max_name_size)
        max_name_size = var_name_size;
    }

    // Perform the output of the variable residuals
    oss << "    |residual|_2 of individual variables:\n";
    for (unsigned int var_num = 0; var_num < _sys.n_vars(); var_num++)
    {
      Real var_res_id = _sys.calculate_norm(*_sys.rhs,var_num,DISCRETE_L2);
      oss << std::setw(27-max_name_size) << " " << std::setw(max_name_size+2) //match position of overall NL residual
          << std::left << _sys.variable_name(var_num) + ":" << var_res_id << "\n";
    }

    _console << oss.str() << std::flush;
  }
}

void
DebugOutput::printMaterialMap() const
{

  // Get a reference to the material warehouse
  MaterialWarehouse & material_warehouse = _problem_ptr->getMaterialWarehouse(0);

  // Build output streams for block materials and block face materials
  std::ostringstream active_block, active_face, active_neighbor;
  std::set<SubdomainID> blocks = material_warehouse.blocks();
  for (std::set<SubdomainID>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
  {
    // Active materials on blocks
    active_block << "  block ID = " << *it << ":\n";
    printMaterialProperties(active_block, material_warehouse.getMaterials(*it));

    // Active face materials on blocks
    active_face << "  block ID = " << *it << ":\n";
    printMaterialProperties(active_face, material_warehouse.getFaceMaterials(*it));

    // Active face materials on blocks
    active_neighbor << "  block ID = " << *it << ":\n";
    printMaterialProperties(active_neighbor, material_warehouse.getNeighborMaterials(*it));
  }

  // Build output stream for side set materials
  std::ostringstream active_boundary;
  std::set<BoundaryID> boundaries = material_warehouse.boundaries();
  for (std::set<BoundaryID>::const_iterator it = boundaries.begin(); it != boundaries.end(); ++it)
  {
    // Active materials on blocks
    active_boundary << "  boundary ID = " << *it << ":\n";
    printMaterialProperties(active_boundary, material_warehouse.getBoundaryMaterials(*it));
  }

  // Write the stored strings to the Console output objects
  _console << "Active materials on blocks:\n" << active_block.str() << '\n';
  _console << "Active face materials on blocks:\n" << active_face.str() << '\n';
  _console << "Active neighboring materials on blocks:\n" << active_neighbor.str() << '\n';
  _console << "Active materials on boundaries:\n" << active_boundary.str() << std::endl;
}


void
DebugOutput::printMaterialProperties( std::ostringstream & output, const std::vector<Material * > & materials) const
{
  for (std::vector<Material *>::const_iterator jt = materials.begin(); jt != materials.end(); ++jt)
  {
    const std::set<std::string> & props = (*jt)->getSuppliedItems();
    output << "    material = " << (*jt)->name() << ":\n";
    for (std::set<std::string>::const_iterator prop_it = props.begin(); prop_it != props.end(); ++prop_it)
      output << "      " << *prop_it << '\n';
  }
}

std::string
DebugOutput::filename()
{
  return _file_base;
}

void
DebugOutput::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for Debug output");
}

void
DebugOutput::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for Debug output");
}

void
DebugOutput::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for Debug output");
}

void
DebugOutput::outputVectorPostprocessors()
{
  mooseError("Individual output of VectorPostprocessors is not support for Debug output");
}

void
DebugOutput::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for Debug output");
}
