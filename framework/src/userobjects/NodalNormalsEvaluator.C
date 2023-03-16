//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalNormalsEvaluator.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "MooseVariableFE.h"
#include "NodalNormalsPreprocessor.h"

registerMooseObject("MooseApp", NodalNormalsEvaluator);

InputParameters
NodalNormalsEvaluator::validParams()
{
  InputParameters params = NodalUserObject::validParams();
  params.addClassDescription(
      "Helper object to compute nodal normal values via the NodalNormals input block.");
  params.set<bool>("_dual_restrictable") = true;
  params.set<std::vector<SubdomainName>>("block") = {"ANY_BLOCK_ID"};
  return params;
}

NodalNormalsEvaluator::NodalNormalsEvaluator(const InputParameters & parameters)
  : NodalUserObject(parameters), _aux(_fe_problem.getAuxiliarySystem())
{
}

void
NodalNormalsEvaluator::execute()
{

  if (_current_node->processor_id() == processor_id())
  {
    if (_current_node->n_dofs(_aux.number(),
                              _fe_problem
                                  .getVariable(_tid,
                                               "nodal_normal_x",
                                               Moose::VarKindType::VAR_AUXILIARY,
                                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                                  .number()) > 0)
    {
      std::scoped_lock lock(NodalNormalsPreprocessor::_nodal_normals_mutex);

      dof_id_type dof_x =
          _current_node->dof_number(_aux.number(),
                                    _fe_problem
                                        .getVariable(_tid,
                                                     "nodal_normal_x",
                                                     Moose::VarKindType::VAR_AUXILIARY,
                                                     Moose::VarFieldType::VAR_FIELD_STANDARD)
                                        .number(),
                                    0);
      dof_id_type dof_y =
          _current_node->dof_number(_aux.number(),
                                    _fe_problem
                                        .getVariable(_tid,
                                                     "nodal_normal_y",
                                                     Moose::VarKindType::VAR_AUXILIARY,
                                                     Moose::VarFieldType::VAR_FIELD_STANDARD)
                                        .number(),
                                    0);
      dof_id_type dof_z =
          _current_node->dof_number(_aux.number(),
                                    _fe_problem
                                        .getVariable(_tid,
                                                     "nodal_normal_z",
                                                     Moose::VarKindType::VAR_AUXILIARY,
                                                     Moose::VarFieldType::VAR_FIELD_STANDARD)
                                        .number(),
                                    0);

      NumericVector<Number> & sln = _aux.solution();
      Real nx = sln(dof_x);
      Real ny = sln(dof_y);
      Real nz = sln(dof_z);

      Real n = std::sqrt((nx * nx) + (ny * ny) + (nz * nz));
      if (std::abs(n) >= 1e-13)
      {
        // divide by n only if it is not close to zero to avoid NaNs
        sln.set(dof_x, nx / n);
        sln.set(dof_y, ny / n);
        sln.set(dof_z, nz / n);
      }
    }
  }
}

void
NodalNormalsEvaluator::initialize()
{
  _aux.solution().close();
}

void
NodalNormalsEvaluator::finalize()
{
  _aux.solution().close();
}

void
NodalNormalsEvaluator::threadJoin(const UserObject & /*uo*/)
{
}
