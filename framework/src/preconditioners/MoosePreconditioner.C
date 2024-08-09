//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MoosePreconditioner.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "NonlinearSystem.h"

#include "libmesh/coupling_matrix.h"
#include "libmesh/numeric_vector.h"

InputParameters
MoosePreconditioner::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addPrivateParam<FEProblemBase *>("_fe_problem_base");

  MooseEnum pc_side("left right symmetric default", "default");
  params.addParam<MooseEnum>("pc_side", pc_side, "Preconditioning side");
  MooseEnum ksp_norm("none preconditioned unpreconditioned natural default", "unpreconditioned");
  params.addParam<MooseEnum>(
      "ksp_norm", ksp_norm, "Sets the norm that is used for convergence testing");
  params.registerBase("MoosePreconditioner");

  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_row",
      "The variable names for the off-diagonal rows you want to add into the matrix; they will be "
      "associated with an off-diagonal column from the same position in off_diag_column.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "off_diag_column",
      "The variable names for the off-diagonal columns you want to add into the matrix; they "
      "will be associated with an off-diagonal row from the same position in off_diag_row.");
  params.addParam<bool>("full",
                        false,
                        "Set to true if you want the full set of couplings between variables "
                        "simply for convenience so you don't have to set every off_diag_row "
                        "and off_diag_column combination.");
  params.addParam<NonlinearSystemName>(
      "nl_sys",
      "The nonlinear system whose linearization this preconditioner should be applied to.");

  params += Moose::PetscSupport::getPetscValidParams();

  return params;
}

MoosePreconditioner::MoosePreconditioner(const InputParameters & params)
  : MooseObject(params),
    Restartable(this, "Preconditioners"),
    PerfGraphInterface(this),
    _fe_problem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _nl_sys_num(
        isParamValid("nl_sys") ? _fe_problem.nlSysNum(getParam<NonlinearSystemName>("nl_sys")) : 0),
    _nl(_fe_problem.getNonlinearSystemBase(_nl_sys_num))
{
  _nl.setPCSide(getParam<MooseEnum>("pc_side"));

  _nl.setMooseKSPNormType(getParam<MooseEnum>("ksp_norm"));

  bool full = getParam<bool>("full");

  // We either have a full coupling or a custom coupling
  if (full && isParamValid("off_diag_row"))
    paramError("off_diag_row", "Set full=false to specify the off-diagonal rows manually");
  if (full && isParamValid("off_diag_column"))
    paramError("off_diag_column", "Set full=false to specify the off-diagonal columns manually");

  // Off-diagonal rows and colums must match
  if (isParamValid("off_diag_row"))
  {
    if (isParamValid("off_diag_column"))
    {
      const auto off_diag =
          getParam<NonlinearVariableName, NonlinearVariableName>("off_diag_row", "off_diag_column");
    }
    else
      paramError("off_diag_row",
                 "If off-diagonal rows are specified, matching off-diagonal "
                 "columns must be specified as well");
  }
  else if (isParamValid("off_diag_column"))
    paramError("off_diag_column",
               "If off-diagonal columns are specified, matching off-diagonal "
               "rows must be specified as well");
}

void
MoosePreconditioner::copyVarValues(MeshBase & mesh,
                                   const unsigned int from_system,
                                   const unsigned int from_var,
                                   const NumericVector<Number> & from_vector,
                                   const unsigned int to_system,
                                   const unsigned int to_var,
                                   NumericVector<Number> & to_vector)
{
  for (auto & node : mesh.local_node_ptr_range())
  {
    unsigned int n_comp = node->n_comp(from_system, from_var);

    mooseAssert(node->n_comp(from_system, from_var) == node->n_comp(to_system, to_var),
                "Number of components does not match in each system");

    for (unsigned int i = 0; i < n_comp; i++)
    {
      dof_id_type from_dof = node->dof_number(from_system, from_var, i);
      dof_id_type to_dof = node->dof_number(to_system, to_var, i);

      to_vector.set(to_dof, from_vector(from_dof));
    }
  }

  for (auto & elem : as_range(mesh.local_elements_begin(), mesh.local_elements_end()))
  {
    unsigned int n_comp = elem->n_comp(from_system, from_var);

    mooseAssert(elem->n_comp(from_system, from_var) == elem->n_comp(to_system, to_var),
                "Number of components does not match in each system");

    for (unsigned int i = 0; i < n_comp; i++)
    {
      dof_id_type from_dof = elem->dof_number(from_system, from_var, i);
      dof_id_type to_dof = elem->dof_number(to_system, to_var, i);

      to_vector.set(to_dof, from_vector(from_dof));
    }
  }
}

void
MoosePreconditioner::setCouplingMatrix(std::unique_ptr<CouplingMatrix> cm)
{
  _fe_problem.setCouplingMatrix(std::move(cm), _nl_sys_num);
}
