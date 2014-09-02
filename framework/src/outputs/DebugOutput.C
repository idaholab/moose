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
#include "Console.h"

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
  params.addParam<unsigned int>("show_top_residuals", 0, "The number of top residuals to print out (0 = no output)");

  // By default operate on linear residuals, this is to maintain the behavior of show_top_residuals
  params.set<bool>("linear_residuals") = true;

  return params;
}

DebugOutput::DebugOutput(const std::string & name, InputParameters & parameters) :
    PetscOutput(name, parameters),
    _show_top_residuals(getParam<unsigned int>("show_top_residuals")),
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
  // Show variable residual norms on Nonlinear iterations
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

  // Display the top residuals
  if (_show_top_residuals > 0)
    printTopResiduals(*(_sys.rhs), _show_top_residuals);
}

void
DebugOutput::printMaterialMap() const
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
DebugOutput::printTopResiduals(const NumericVector<Number> & residual, unsigned int n)
{
  // Need a reference to the libMesh mesh object
  MeshBase & mesh = _problem_ptr->mesh().getMesh();

  std::vector<DebugOutputTopResidualData> vec;
  vec.resize(residual.local_size());

  unsigned int j = 0;
  MeshBase::node_iterator it = mesh.local_nodes_begin();
  const MeshBase::node_iterator end = mesh.local_nodes_end();
  for (; it != end; ++it)
  {
    Node & node = *(*it);
    dof_id_type nd = node.id();

    for (unsigned int var = 0; var < node.n_vars(_sys.number()); ++var)
      if (node.n_dofs(_sys.number(), var) > 0)
      {
        dof_id_type dof_idx = node.dof_number(_sys.number(), var, 0);
        vec[j] = DebugOutputTopResidualData(var, nd, residual(dof_idx));
        j++;
      }
  }

  // Sort vec by residuals
  std::sort(vec.begin(), vec.end(), sortTopResidualData);

  // Display the residuals
  Moose::err << "[DBG][" << processor_id() << "] Max " << n << " residuals";
  if (j < n)
  {
    n = j;
    Moose::err << " (Only " << n << " available)";
  }
  Moose::err << std::endl;

  for (unsigned int i = 0; i < n; ++i)
    fprintf(stderr, "[DBG][%d]  % .15e '%s' at node %d\n", processor_id(), vec[i]._residual, _sys.variable_name(vec[i]._var).c_str(), vec[i]._nd);
}

void
DebugOutput::printMaterialProperties(std::stringstream & output, const std::vector<Material * > & materials) const
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
