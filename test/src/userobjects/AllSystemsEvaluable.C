//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllSystemsEvaluable.h"
#include "SubProblem.h"
#include "MooseMesh.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"
#include "libmesh/equation_systems.h"
#include "libmesh/system.h"
#include "libmesh/dof_map.h"
#include "libmesh/numeric_vector.h"

registerMooseObject("MooseTestApp", AllSystemsEvaluable);

InputParameters
AllSystemsEvaluable::validParams()
{
  auto params = GeneralUserObject::validParams();
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

AllSystemsEvaluable::AllSystemsEvaluable(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
AllSystemsEvaluable::execute()
{
  const MeshBase & mesh = _subproblem.mesh().getMesh();
  const EquationSystems & eq = _subproblem.es();
  auto n_sys = eq.n_systems();
  std::vector<dof_id_type> dof_indices;
  std::vector<Number> values;

  for (const auto & elem : mesh.active_local_element_ptr_range())
    for (const auto & neighbor : elem->neighbor_ptr_range())
      // Are we non-local?
      if (neighbor && neighbor->processor_id() != this->processor_id())
        for (MooseIndex(n_sys) sys_num = 0; sys_num < n_sys; ++sys_num)
        {
          const System & system = eq.get_system(sys_num);
          const DofMap & dof_map = system.get_dof_map();

          auto n_vars = dof_map.n_variables();
          for (MooseIndex(n_vars) var_num = 0; var_num < n_vars; ++var_num)
          {
            dof_map.dof_indices(neighbor, dof_indices, var_num);
            system.current_local_solution->get(dof_indices, values);
          }
        }
}
