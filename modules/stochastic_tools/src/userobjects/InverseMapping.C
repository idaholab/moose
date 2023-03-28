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

  params.addParam<std::vector<UserObjectName>>(
      "surrogate", std::vector<UserObjectName>(), "Blabla.");
  params.addRequiredParam<UserObjectName>("mapping", "Blabla.");
  params.addRequiredParam<std::vector<VariableName>>("variable_to_fill", "Blabla.");
  params.addRequiredParam<std::vector<VariableName>>("variable_to_reconstruct", "Blabla");
  params.addRequiredParam<std::vector<Real>>("parameters", "Blabla");
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

  if (_surrogate_model_names.size())
  {
    _surrogate_models.clear();
    for (const auto & name : _surrogate_model_names)
    {
      _surrogate_models.push_back(&getSurrogateModelByName(name));
    }
  }

  _variable_to_fill.clear();
  _variable_to_reconstruct.clear();
  _is_nodal.clear();

  const auto & mapping_variable_names = _mapping->getVariableNames();

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
      paramError("fe_type_fill",
                 "The FEtype should match the ones defined for `variable_to_reconstruct`");

    _is_nodal.push_back(fe_type_fill.family == LAGRANGE);
  }
}

void
InverseMapping::execute()
{

  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();

  std::unique_ptr<NumericVector<Number>> temporary_vector = nl.solution().zero_clone();

  for (auto var_i : index_range(_var_names_to_reconstruct))
  {
    std::vector<Real> reduced_coefficients;
    if (_surrogate_models.size())
    {
      _surrogate_models[var_i]->evaluate(_input_parameters, reduced_coefficients);
    }
    else
    {
      reduced_coefficients = _input_parameters;
    }

    DenseVector<Real> reconstructed_solution;
    _mapping->inverse_map(
        _var_names_to_reconstruct[var_i], reduced_coefficients, reconstructed_solution);

    MooseVariableFieldBase * var_to_fill = _variable_to_fill[var_i];
    const MooseVariableFieldBase * var_to_reconstruct = _variable_to_reconstruct[var_i];

    nl.setVariableGlobalDoFs(_var_names_to_reconstruct[var_i]);

    const auto & dofs = nl.getVariableGlobalDoFs();

    const auto & dof_map = var_to_reconstruct->sys().system().get_dof_map();

    dof_id_type local_dof_begin = dof_map.first_dof();
    dof_id_type local_dof_end = dof_map.end_dof();

    for (const auto & dof_i : index_range(dofs))
    {
      if (dofs[dof_i] >= local_dof_begin && dofs[dof_i] < local_dof_end)
      {
        temporary_vector->set(dofs[dof_i], reconstructed_solution(dof_i));
      }
    }

    unsigned int to_sys_num = _variable_to_fill[var_i]->sys().system().number();
    unsigned int to_var_num =
        var_to_fill->sys().system().variable_number(_var_names_to_fill[var_i]);

    unsigned int from_sys_num = var_to_reconstruct->sys().system().number();
    unsigned int from_var_num =
        var_to_reconstruct->sys().system().variable_number(_var_names_to_reconstruct[var_i]);
    const MeshBase & to_mesh = _fe_problem.mesh().getMesh();

    if (_is_nodal[var_i])
    {

      for (const auto & node : to_mesh.local_node_ptr_range())
      {
        if (node->n_dofs(to_sys_num, to_var_num) < 1)
          continue;

        const auto & to_dof_id = node->dof_number(to_sys_num, to_var_num, 0);
        const auto & from_dof_id = node->dof_number(from_sys_num, from_var_num, 0);

        var_to_fill->sys().solution().set(to_dof_id, (*temporary_vector)(from_dof_id));
      }
    }
    else
    {
      for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
      {
        const auto n_dofs = elem->n_dofs(to_sys_num, to_var_num);
        if (n_dofs < 1)
          continue;

        for (auto dof_i : make_range(n_dofs))
        {
          const auto & to_dof_id = elem->dof_number(to_sys_num, to_var_num, dof_i);
          const auto & from_dof_id = elem->dof_number(from_sys_num, from_var_num, dof_i);

          var_to_fill->sys().solution().set(to_dof_id, (*temporary_vector)(from_dof_id));
        }
      }
    }
    var_to_fill->sys().solution().close();
  }
}
