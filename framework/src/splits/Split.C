//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Split.h"
#include "InputParameters.h"
#include "PetscSupport.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", Split);

InputParameters
Split::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addClassDescription("Field split based preconditioner for nonlinear solver.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "vars", {}, "Variables Split operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<SubdomainName>>(
      "blocks", {}, "Mesh blocks Split operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<BoundaryName>>(
      "sides", {}, "Sidesets Split operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<BoundaryName>>(
      "unsides",
      {},
      "Sidesets Split excludes (omitting this implies \"do not exclude any sidesets\"");
  params.addParam<std::vector<std::string>>(
      "splitting", {}, "The names of the splits (subsystems) in the decomposition of this split");
  params.addParam<std::vector<BoundaryName>>(
      "unside_by_var_boundary_name",
      "A map from boundary name to unside by variable, e.g. only unside for a given variable.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "unside_by_var_var_name",
      "A map from boundary name to unside by variable, e.g. only unside for a given variable.");

  MooseEnum SplittingTypeEnum("additive multiplicative symmetric_multiplicative schur", "additive");
  params.addParam<MooseEnum>("splitting_type", SplittingTypeEnum, "Split decomposition type");

  MooseEnum SchurTypeEnum("full upper lower", "full");
  params.addParam<MooseEnum>("schur_type", SchurTypeEnum, "Type of Schur complement");

  /**
   * Which preconditioning matrix to use with S = D - CA^{-1}B
   * 'Self' means use S to build the preconditioner.
   * limited choices here: PCNONE and PCLSC in PETSc
   * 'D' means the lower-right block in splitting J = [A B; C D]
   */
  MooseEnum SchurPreEnum("S Sp A11", "S");
  params.addParam<MooseEnum>(
      "schur_pre", SchurPreEnum, "Type of Schur complement preconditioner matrix");

  params.addParam<MultiMooseEnum>("petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string>>("petsc_options_iname",
                                            "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string>>("petsc_options_value",
                                            "PETSc option values for the FieldSplit solver");

  params.registerBase("Split");
  return params;
}

Split::Split(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(this, "Splits"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vars(getParam<std::vector<NonlinearVariableName>>("vars")),
    _blocks(getParam<std::vector<SubdomainName>>("blocks")),
    _sides(getParam<std::vector<BoundaryName>>("sides")),
    _unsides(getParam<std::vector<BoundaryName>>("unsides")),
    _splitting(getParam<std::vector<std::string>>("splitting")),
    _splitting_type(getParam<MooseEnum>("splitting_type")),
    _schur_type(getParam<MooseEnum>("schur_type")),
    _schur_pre(getParam<MooseEnum>("schur_pre"))
{
  _petsc_options.flags = getParam<MultiMooseEnum>("petsc_options");
  _petsc_options.pairs =
      getParam<std::string, std::string>("petsc_options_iname", "petsc_options_value");
}

void
Split::setup(NonlinearSystemBase & nl, const std::string & prefix)
{
  // The Split::setup() implementation does not actually depend on any
  // specific version of PETSc, so there's no need to wrap the entire
  // function.

  // A reference to the PetscOptions
  Moose::PetscSupport::PetscOptions & po = _fe_problem.getPetscOptions();
  // prefix
  std::string dmprefix = prefix + "dm_moose_";

  if (isParamValid("unside_by_var_boundary_name"))
  {
    const auto & unside_by_var_boundary_name =
        getParam<std::vector<BoundaryName>>("unside_by_var_boundary_name");
    const auto & unside_by_var_var_name =
        getParam<std::vector<NonlinearVariableName>>("unside_by_var_var_name");

    std::vector<std::string> vector_of_pairs;
    for (const auto i : index_range(unside_by_var_boundary_name))
      vector_of_pairs.push_back(unside_by_var_boundary_name[i] + ":" + unside_by_var_var_name[i]);
    po.pairs.emplace_back(dmprefix + "unside_by_var", Moose::stringify(vector_of_pairs, ","));
  }

  // var options
  if (!_vars.empty())
  {
    po.pairs.emplace_back(dmprefix + "vars", Moose::stringify(_vars, ","));

    // check that variables are either field or scalars
    for (const auto & var : _vars)
      if (!_fe_problem.hasVariable(var) && !_fe_problem.hasScalarVariable(var))
        mooseError("Variable '", var, "' specified in split '", name(), "' does not exist");
  }

  // block options
  if (!_blocks.empty())
    po.pairs.emplace_back(dmprefix + "blocks", Moose::stringify(_blocks, ","));

  // side options
  if (!_sides.empty())
    po.pairs.emplace_back(dmprefix + "sides", Moose::stringify(_sides, ","));

  // unside options
  if (!_unsides.empty())
    po.pairs.emplace_back(dmprefix + "unsides", Moose::stringify(_unsides, ","));

  if (!_splitting.empty())
  {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's
    // subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    po.pairs.emplace_back(prefix + "pc_type", "fieldsplit");

    // set Splitting Type
    const std::string petsc_splitting_type[] = {
        "additive", "multiplicative", "symmetric_multiplicative", "schur"};
    po.pairs.emplace_back(prefix + "pc_fieldsplit_type", petsc_splitting_type[_splitting_type]);

    if (_splitting_type == SplittingTypeSchur)
    {
      // set Schur Type
      const std::string petsc_schur_type[] = {"diag", "upper", "lower", "full"};
      po.pairs.emplace_back(prefix + "pc_fieldsplit_schur_fact_type",
                            petsc_schur_type[_splitting_type]);

      // set Schur Preconditioner
      const std::string petsc_schur_pre[] = {"self", "selfp", "a11"};
      po.pairs.emplace_back(prefix + "pc_fieldsplit_schur_precondition",
                            petsc_schur_pre[_schur_pre]);
    }

    // The DM associated with this split defines the subsplits' geometry.
    po.pairs.emplace_back(dmprefix + "nfieldsplits", Moose::stringify(_splitting.size()));
    po.pairs.emplace_back(dmprefix + "fieldsplit_names", Moose::stringify(_splitting, ","));

    // Finally, recursively configure the splits contained within this split.
    for (const auto & split_name : _splitting)
    {
      std::shared_ptr<Split> split = nl.getSplit(split_name);
      std::string sprefix = prefix + "fieldsplit_" + split_name + "_";
      split->setup(nl, sprefix);
    }
  }

  // Now we set the user-specified petsc options for this split, possibly overriding the above
  // settings.
  for (const auto & item : _petsc_options.flags)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    std::string op(item);
    if (op[0] != '-')
      mooseError("Invalid PETSc option name ", op, " for Split ", _name);

    // push back PETSc options
    po.flags.setAdditionalValue(prefix + op.substr(1));
  }

  for (auto & option : _petsc_options.pairs)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = option.first;
    if (op[0] != '-')
      mooseError("Invalid PETSc option name ", op, " for Split ", _name);

    po.pairs.emplace_back(prefix + op.substr(1), option.second);
  }
}
