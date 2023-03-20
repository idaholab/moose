//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADUtils.h"

#include "NonlinearSystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MooseError.h"
#include "libmesh/system.h"
#include "libmesh/dof_map.h"

namespace Moose
{

std::unordered_map<dof_id_type, Real>
globalDofIndexToDerivative(const ADReal & ad_real,
                           const SystemBase & sys,
                           const ElementType elem_type /*=ElementType::Element*/,
                           const THREAD_ID tid /*=0*/)
{
  mooseAssert(dynamic_cast<const NonlinearSystemBase *>(&sys),
              "This must be a nonlinear system base object");
  const Assembly & assembly = sys.subproblem().assembly(tid, sys.number());
  const Elem * elem;
  switch (elem_type)
  {
    case ElementType::Element:
      elem = assembly.elem();
      break;

    case ElementType::Neighbor:
      elem = assembly.neighbor();
      break;

    case ElementType::Lower:
      elem = assembly.lowerDElem();
      break;

    default:
      mooseError("Unrecognized element type");
  }

  std::unordered_map<dof_id_type, Real> ret_val;

  const System & libmesh_sys = sys.system();
  const DofMap & dof_map = libmesh_sys.get_dof_map();

  const unsigned int num_vars = libmesh_sys.n_vars();

  const auto max_dofs_per_elem = sys.getMaxVarNDofsPerElem();

  for (unsigned int var_num = 0; var_num < num_vars; ++var_num)
  {
    std::vector<dof_id_type> global_indices;

    // Get the global indices corresponding to var_num that exist on elem
    dof_map.dof_indices(elem, global_indices, var_num);

    // determine the AD offset for the current var
    const auto ad_offset = adOffset(var_num, max_dofs_per_elem, elem_type, num_vars);

    // Map from global index to derivative
    for (MooseIndex(global_indices) local_index = 0; local_index < global_indices.size();
         ++local_index)
      ret_val[global_indices[local_index]] = ad_real.derivatives()[ad_offset + local_index];
  }

  return ret_val;
}

}
