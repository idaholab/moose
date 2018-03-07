//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorToAuxVar.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MultiApp.h"
#include "DelimitedFileReader.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"
#include "libmesh/parallel_algebra.h"

template <>
InputParameters
validParams<VectorPostprocessorToAuxVar>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variables",
      "The vector of auxiliary variables to store the transferred values in master model.");
  params.addRequiredParam<VectorPostprocessorName>(
      "vector_postprocessor", "The vectorprocessor that contains the data in sub applications.");
  params.addRequiredParam<std::vector<std::string>>(
      "variable_vector_names",
      "The names of the variable vector that need to be transferred from "
      "the vector postprocessor to aux variables in master app.");
  params.addParam<std::vector<SubdomainName>>(
      "master_mesh_block_ids",
      "If provided the vector_postprocessor values would be transferred "
      "only to nodes belonging to those blocks.");
  params.addParam<unsigned int>(
      "master_component", 2, "Component along which interpolation occurs in the master model.");
  params.addParam<unsigned int>(
      "subapp_component", 1, "Component along which interpolation occurs in the SubApp.");
  params.addRequiredParam<FileName>(
      "positions",
      "The file that contains positions of the beams in directions other than "
      "master_component provided using same numbering scheme as the subapp. For "
      "example, if master_component is 1, then the first column of the file would "
      "contain x poisiton and the second column would contain the z position.");
  return params;
}

VectorPostprocessorToAuxVar::VectorPostprocessorToAuxVar(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _vector_postprocessor(getParam<VectorPostprocessorName>("vector_postprocessor")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variables")),
    _variable_vector_names(getParam<std::vector<std::string>>("variable_vector_names")),
    _has_blocks(isParamValid("master_mesh_block_ids")),
    _master_component(getParam<unsigned int>("master_component")),
    _subapp_component(getParam<unsigned int>("subapp_component")),
    _horizontal_location_1(_multi_app->numGlobalApps()),
    _horizontal_location_2(_multi_app->numGlobalApps())
{
  MooseUtils::DelimitedFileReader positions(getParam<FileName>("positions"));
  positions.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
  positions.read();
  _horizontal_location_1 = positions.getData("column_0");
  _horizontal_location_2 = positions.getData("column_1");

  if (_to_var_names.size() != _variable_vector_names.size())
    mooseError("Vector Postprocessor To AuxVar Transfer: variables and variable_vector_names must "
               "be of the same size");
}

void
VectorPostprocessorToAuxVar::execute()
{
  _console << "Beginning Vector Postprocessor To AuxVar Transfer " << name() << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      mooseError("Vector Postprocessor To AuxVar Transfer: Can't transfer from vector "
                 "postprocesser in master app to auxvariable in subapp.");
      break;
    }
    case FROM_MULTIAPP:
    {
      std::vector<std::vector<VectorPostprocessorValue>> src_values(_multi_app->numGlobalApps());
      std::vector<VectorPostprocessorValue> vertical_location(_multi_app->numGlobalApps());

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        std::vector<VectorPostprocessorValue> src_val_app(_variable_vector_names.size());
        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
        {
          if (_subapp_component == 0)
            vertical_location[i] = _multi_app->appProblemBase(i).getVectorPostprocessorValue(
                _vector_postprocessor, "x");
          else if (_subapp_component == 1)
            vertical_location[i] = _multi_app->appProblemBase(i).getVectorPostprocessorValue(
                _vector_postprocessor, "y");
          else
            vertical_location[i] = _multi_app->appProblemBase(i).getVectorPostprocessorValue(
                _vector_postprocessor, "z");

          for (unsigned int j = 0; j < _variable_vector_names.size(); ++j)
            src_val_app[j] = _multi_app->appProblemBase(i).getVectorPostprocessorValue(
                _vector_postprocessor, _variable_vector_names[0]);
        }
        _communicator.allgather(vertical_location[i]);
        for (unsigned int j = 0; j < _variable_vector_names.size(); ++j)
          _communicator.allgather(src_val_app[j]);
        src_values[i] = src_val_app;
      }

      // Loop over the master nodes and set the value of the variables
      {
        System * to_sys = find_sys(_multi_app->problemBase().es(), _to_var_names[0]);

        unsigned int sys_num = to_sys->number();
        std::vector<unsigned int> var_num(_to_var_names.size());
        for (unsigned int i = 0; i < _to_var_names.size(); ++i)
          var_num[i] = to_sys->variable_number(_to_var_names[i]);

        NumericVector<Real> & solution = *to_sys->solution;

        MooseMesh & mesh = _multi_app->problemBase().mesh();

        MeshBase::const_node_iterator node_it = mesh.localNodesBegin();
        MeshBase::const_node_iterator node_end = mesh.localNodesEnd();

        std::vector<unsigned int> master_horizontal_components;
        for (unsigned int i = 0; i < 3; ++i)
          if (i != _master_component)
            master_horizontal_components.push_back(i);

        std::vector<Real> vals(_to_var_names.size());
        for (; node_it != node_end; ++node_it)
        {
          Node * node = *node_it;

          // If master_mesh_block_ids are provided as input, then iterate only over nodes belonging
          // to this block
          if (_has_blocks)
          {
            std::vector<SubdomainID> master_blocks =
                mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("master_mesh_block_ids"));
            std::sort(master_blocks.begin(), master_blocks.end());
            std::set<SubdomainID> node_blocks = mesh.getNodeBlockIds(*node);
            std::vector<SubdomainID> common_blocks;
            std::set_intersection(master_blocks.begin(),
                                  master_blocks.end(),
                                  node_blocks.begin(),
                                  node_blocks.end(),
                                  std::back_inserter(common_blocks));
            if (common_blocks.size() == 0)
              continue;
          }

          // check whether any of the aux variables have dofs at this node
          bool variable_has_dofs = false;
          for (unsigned int i = 0; i < _to_var_names.size(); ++i)
            if (node->n_dofs(sys_num, var_num[i]) > 0)
            {
              variable_has_dofs = true;
              break;
            }

          if (variable_has_dofs) // If any aux variable has dofs at this node
          {
            Point pts = *node;
            std::fill(vals.begin(), vals.end(), 0.0);

            for (unsigned int i = 0; i < _horizontal_location_1.size(); ++i)
            {
              if (std::abs(_horizontal_location_1[i] - pts(master_horizontal_components[0])) <
                      1e-6 &&
                  std::abs(_horizontal_location_2[i] - pts(master_horizontal_components[1])) < 1e-6)
              {
                auto lower = std::lower_bound(vertical_location[i].begin(),
                                              vertical_location[i].end(),
                                              pts(_master_component));
                unsigned int index = std::distance(vertical_location[i].begin(), lower);

                if (lower != vertical_location[i].end() &&
                    pts(_master_component) - vertical_location[i][0] > -1e-6 &&
                    vertical_location[i].back() - pts(_master_component) > -1e-6)
                {
                  if (std::abs(*lower - pts(_master_component)) < 1e-6)
                    for (unsigned int j = 0; j < _to_var_names.size(); ++j)
                      vals[j] = src_values[i][j][index];
                  else
                  {
                    Real scaling_factor =
                        (pts(_master_component) - vertical_location[i][index - 1]) /
                        (vertical_location[i][index] - vertical_location[i][index - 1]);
                    for (unsigned int j = 0; j < _to_var_names.size(); ++j)
                      vals[j] =
                          src_values[i][j][index - 1] +
                          (src_values[i][j][index] - src_values[i][j][index - 1]) * scaling_factor;
                  }
                }

                else if (pts(_master_component) - vertical_location[i][0] < -1e-6)
                {
                  auto lower = std::upper_bound(vertical_location[i].begin(),
                                                vertical_location[i].end(),
                                                vertical_location[i][0]);
                  unsigned int index = std::distance(vertical_location[i].begin(), lower);
                  Real scaling_factor = (pts(_master_component) - vertical_location[i][0]) /
                                        (*lower - vertical_location[i][0]);
                  for (unsigned int j = 0; j < _to_var_names.size(); ++j)
                    vals[j] = src_values[i][j][0] +
                              (src_values[i][j][index] - src_values[i][j][0]) * scaling_factor;
                }

                else if (vertical_location[i].back() - pts(_master_component) < -1e-6)
                {
                  auto lower = std::lower_bound(vertical_location[i].begin(),
                                                vertical_location[i].end(),
                                                vertical_location[i].back());
                  unsigned int index = std::distance(vertical_location[i].begin(), lower);

                  Real scaling_factor =
                      (pts(_master_component) - vertical_location[i].back()) /
                      (vertical_location[i][index - 1] - vertical_location[i].back());
                  for (unsigned int j = 0; j < _to_var_names.size(); ++j)
                    vals[j] =
                        src_values[i][j].back() +
                        (src_values[i][j][index - 1] - src_values[i][j].back()) * scaling_factor;
                }

                else
                  mooseError(
                      "Vector Postprocessor To AuxVar Transfer: Should not be getting here!");
              }
            }

            // The zero only works for LAGRANGE!
            for (unsigned int i = 0; i < _to_var_names.size(); ++i)
            {
              if (node->n_dofs(sys_num, var_num[i]) > 0)
              {
                dof_id_type dof = node->dof_number(sys_num, var_num[i], 0);

                solution.set(dof, vals[i]);
              }
            }
          }
        }

        solution.close();
      }

      _multi_app->problemBase().es().update();

      break;
    }
  }

  _console << "Finished Vector Postprocessor To AuxVar Transfer " << name() << std::endl;
}
