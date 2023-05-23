//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InverseMapping.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

#include "libmesh/sparse_matrix.h"

registerMooseObject("StochasticToolsApp", InverseMapping);

InputParameters
InverseMapping::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addClassDescription("Evaluates surrogate models and maps the results back to a full "
                             "solution field for given variables.");
  params.addParam<std::vector<UserObjectName>>(
      "surrogate", std::vector<UserObjectName>(), "The names of the surrogates for each variable.");
  params.addRequiredParam<UserObjectName>(
      "mapping", "The name of the mapping object which provides the inverse mapping function.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variable_to_fill",
      "The names of the variables that this object is supposed to populate with the "
      "reconstructed results.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variable_to_reconstruct",
      "The names of the variables in the nonlinear system which we would like to approximate. This "
      "is important for DoF information.");
  params.addRequiredParam<std::vector<Real>>(
      "parameters",
      "The input parameters for the surrogate. If no surrogate is supplied these are assumed to be "
      "the coordinates in the latent space.");
  params.declareControllable("parameters");

  return params;
}

InverseMapping::InverseMapping(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    MappingInterface(this),
    SurrogateModelInterface(this),
    _var_names_to_fill(getParam<std::vector<VariableName>>("variable_to_fill")),
    _var_names_to_reconstruct(getParam<std::vector<VariableName>>("variable_to_reconstruct")),
    _surrogate_model_names(getParam<std::vector<UserObjectName>>("surrogate")),
    _input_parameters(getParam<std::vector<Real>>("parameters"))
{
  if (_var_names_to_fill.size() != _var_names_to_reconstruct.size())
    paramError("variable_to_fill",
               "The number of variables to fill should be the same as the number of entries in "
               "`variable_to_reconstruct`");
  if (_surrogate_model_names.size())
  {
    if (_var_names_to_fill.size() != _surrogate_model_names.size())
      paramError("surrogate",
                 "The number of surrogates should match the number of variables which need to be "
                 "reconstructed!");
  }
}

void
InverseMapping::initialSetup()
{
  _mapping = &getMapping("mapping");

  _surrogate_models.clear();
  for (const auto & name : _surrogate_model_names)
    _surrogate_models.push_back(&getSurrogateModelByName(name));

  _variable_to_fill.clear();
  _variable_to_reconstruct.clear();

  const auto & mapping_variable_names = _mapping->getVariableNames();

  // We query the links to the MooseVariables mentioned in the input parameters
  for (const auto & var_i : index_range(_var_names_to_reconstruct))
  {
    if (std::find(mapping_variable_names.begin(),
                  mapping_variable_names.end(),
                  _var_names_to_reconstruct[var_i]) == mapping_variable_names.end())
      paramError("variable_to_reconstruct",
                 "Couldn't find mapping for " + _var_names_to_reconstruct[var_i] +
                     "! Double check the training process and make sure that the mapping includes "
                     "the given variable!");

    _variable_to_fill.push_back(&_fe_problem.getVariable(_tid, _var_names_to_fill[var_i]));
    _variable_to_reconstruct.push_back(
        &_fe_problem.getVariable(_tid, _var_names_to_reconstruct[var_i]));

    auto & fe_type_reconstruct = _variable_to_reconstruct.back()->feType();
    auto & fe_type_fill = _variable_to_fill.back()->feType();

    if (fe_type_reconstruct != fe_type_fill)
      paramError("variable_to_fill",
                 "The FEtype should match the ones defined for `variable_to_reconstruct`");

    if (fe_type_reconstruct.family == SCALAR)
      paramError("variable_to_fill", "InverseMapping does not support SCALAR variables!");
  }
}

void
InverseMapping::execute()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();

  // We create a temporary solution vector that will store the reconstructed solution.
  std::unique_ptr<NumericVector<Number>> temporary_vector = nl.solution().zero_clone();

  for (auto var_i : index_range(_var_names_to_reconstruct))
  {
    std::vector<Real> reduced_coefficients;
    if (_surrogate_models.size())
      _surrogate_models[var_i]->evaluate(_input_parameters, reduced_coefficients);
    else
      reduced_coefficients = _input_parameters;

    // The sanity check on the size of reduced_coefficients is happening in the inverse mapping
    // function
    DenseVector<Real> reconstructed_solution;
    _mapping->inverse_map(
        _var_names_to_reconstruct[var_i], reduced_coefficients, reconstructed_solution);

    MooseVariableFieldBase * var_to_fill = _variable_to_fill[var_i];
    const MooseVariableFieldBase * var_to_reconstruct = _variable_to_reconstruct[var_i];

    // We set the global DoF indices of the requested variable. The underlying assumption here is
    // that we are reconstructing the solution in an application which has the same ordering in the
    // extracted `dofs` vector as the one which was used to serialize the solution vectors in
    // `SerializedSolutionTransfer`.
    nl.setVariableGlobalDoFs(_var_names_to_reconstruct[var_i]);

    // Get the DoF indices
    const auto & dofs = nl.getVariableGlobalDoFs();

    // Get the dof map to be able to determine the local dofs easily
    const auto & dof_map = var_to_reconstruct->sys().system().get_dof_map();
    dof_id_type local_dof_begin = dof_map.first_dof();
    dof_id_type local_dof_end = dof_map.end_dof();

    // Populate the temporary vector with the reconstructed solution
    for (const auto & dof_i : index_range(dofs))
      if (dofs[dof_i] >= local_dof_begin && dofs[dof_i] < local_dof_end)
        temporary_vector->set(dofs[dof_i], reconstructed_solution(dof_i));

    // Get the system and variable numbers for the dof objects
    unsigned int to_sys_num = _variable_to_fill[var_i]->sys().system().number();
    unsigned int to_var_num =
        var_to_fill->sys().system().variable_number(_var_names_to_fill[var_i]);

    unsigned int from_sys_num = var_to_reconstruct->sys().system().number();
    unsigned int from_var_num =
        var_to_reconstruct->sys().system().variable_number(_var_names_to_reconstruct[var_i]);

    // Get a link to the mesh for the loops over dof objects
    const MeshBase & to_mesh = _fe_problem.mesh().getMesh();

    // First, we cover nodal degrees of freedom.
    for (const auto & node : to_mesh.local_node_ptr_range())
    {
      const auto n_dofs = node->n_dofs(to_sys_num, to_var_num);
      // We have nothing to do if we don't have dofs at this node
      if (n_dofs < 1)
        continue;

      // For special cases we might have multiple dofs for the same node (hierarchic types)
      for (auto dof_i : make_range(n_dofs))
      {
        // Get the dof ids for the from/to variables
        const auto & to_dof_id = node->dof_number(to_sys_num, to_var_num, dof_i);
        const auto & from_dof_id = node->dof_number(from_sys_num, from_var_num, dof_i);

        // Fill the dof of the variable using the dof of the temporary variable
        var_to_fill->sys().solution().set(to_dof_id, (*temporary_vector)(from_dof_id));
      }
    }

    // Then we move on to element-based degrees of freedom. Considering that we don't
    // support scalar variables at the moment, this should take care of every remaining
    // local entry. Of course, we'll only need this part if the variable is not nodal.
    if (!_variable_to_reconstruct[var_i]->isNodal())
      for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        // Check how many dofs we have on the current element, if none we have nothing to do
        const auto n_dofs = elem->n_dofs(to_sys_num, to_var_num);
        if (n_dofs < 1)
          continue;

        // Loop over the dofs and populate the variable
        for (auto dof_i : make_range(n_dofs))
        {
          // Get the dof for the from/to variables
          const auto & to_dof_id = elem->dof_number(to_sys_num, to_var_num, dof_i);
          const auto & from_dof_id = elem->dof_number(from_sys_num, from_var_num, dof_i);

          var_to_fill->sys().solution().set(to_dof_id, (*temporary_vector)(from_dof_id));
        }
      }

    // Close the solution to make sure we can output the variable
    var_to_fill->sys().solution().close();
  }
}
