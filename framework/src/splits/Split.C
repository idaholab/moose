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

template <>
InputParameters
validParams<Split>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::vector<NonlinearVariableName>>(
      "vars", "Variables Split operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<SubdomainName>>(
      "blocks", "Mesh blocks Split operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<BoundaryName>>(
      "sides", "Sidesets Split operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<BoundaryName>>(
      "unsides", "Sidesets Split excludes (omitting this implies \"do not exclude any sidesets\"");
  params.addParam<std::vector<std::string>>(
      "splitting", "The names of the splits (subsystems) in the decomposition of this split");

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

  MooseEnum SchurAInvEnum("diag lump", "diag");
  params.addParam<MooseEnum>(
      "schur_ainv",
      SchurAInvEnum,
      "Type of approximation to inv(A) used when forming S = D - C inv(A) B");

#if LIBMESH_HAVE_PETSC
  params.addParam<MultiMooseEnum>("petsc_options",
                                  Moose::PetscSupport::getCommonPetscFlags(),
                                  "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string>>("petsc_options_iname",
                                            "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string>>("petsc_options_value",
                                            "PETSc option values for the FieldSplit solver");
#endif

  params.registerBase("Split");
  return params;
}

Split::Split(const InputParameters & parameters)
  : MooseObject(parameters),
    Restartable(parameters, "Splits"),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _vars(getParam<std::vector<NonlinearVariableName>>("vars")),
    _blocks(getParam<std::vector<SubdomainName>>("blocks")),
    _sides(getParam<std::vector<BoundaryName>>("sides")),
    _unsides(getParam<std::vector<BoundaryName>>("unsides")),
    _splitting(getParam<std::vector<std::string>>("splitting")),
    _splitting_type(getParam<MooseEnum>("splitting_type")),
    _schur_type(getParam<MooseEnum>("schur_type")),
    _schur_pre(getParam<MooseEnum>("schur_pre")),
    _schur_ainv(getParam<MooseEnum>("schur_ainv"))
{
  _petsc_options.flags = getParam<MultiMooseEnum>("petsc_options");
  _petsc_options.inames = getParam<std::vector<std::string>>("petsc_options_iname");
  _petsc_options.values = getParam<std::vector<std::string>>("petsc_options_value");
}

void
Split::setup(const std::string & prefix)
{
// petsc 3.3.0 or later needed
#if !defined(LIBMESH_HAVE_PETSC) || PETSC_VERSION_LESS_THAN(3, 3, 0)
  mooseError("The Splits functionality requires PETSc 3.3.0 or later.");
#endif

  // The Split::setup() implementation does not actually depend on any
  // specific version of PETSc, so there's no need to wrap the entire
  // function.

  // A reference to the PetscOptions
  Moose::PetscSupport::PetscOptions & po = _fe_problem.getPetscOptions();
  // prefix
  std::string dmprefix = prefix + "dm_moose_";

  // var options
  if (!_vars.empty())
  {
    po.inames.push_back(dmprefix + "vars");
    po.values.push_back(Moose::stringify(_vars));

    for (const auto & var : _vars)
      if (!_fe_problem.hasVariable(var))
        mooseError("Variable '", var, "' specified in split '", name(), "' does not exist");
  }

  // block options
  if (!_blocks.empty())
  {
    po.inames.push_back(dmprefix + "blocks");
    po.values.push_back(Moose::stringify(_blocks));
  }

  // side options
  if (!_sides.empty())
  {
    po.inames.push_back(dmprefix + "sides");
    po.values.push_back(Moose::stringify(_sides));
  }

  // unside options
  if (!_unsides.empty())
  {
    po.inames.push_back(dmprefix + "unsides");
    po.values.push_back(Moose::stringify(_unsides));
  }

  if (!_splitting.empty())
  {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's
    // subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    po.inames.push_back(prefix + "pc_type");
    po.values.push_back("fieldsplit");

    // set Splitting Type
    const std::string petsc_splitting_type[] = {
        "additive", "multiplicative", "symmetric_multiplicative", "schur"};
    po.inames.push_back(prefix + "pc_fieldsplit_type");
    po.values.push_back(petsc_splitting_type[_splitting_type]);

    if (_splitting_type == SplittingTypeSchur)
    {
      // set Schur Type
      const std::string petsc_schur_type[] = {"diag", "upper", "lower", "full"};
      po.inames.push_back(prefix + "pc_fieldsplit_schur_fact_type");
      po.values.push_back(petsc_schur_type[_splitting_type]);

      // set Schur Preconditioner
      const std::string petsc_schur_pre[] = {
        "self",
        "selfp",
#if PETSC_VERSION_LESS_THAN(3, 4, 0)
        "diag"
#else
        "a11"
#endif
      };
      po.inames.push_back(prefix + "pc_fieldsplit_schur_precondition");
      po.values.push_back(petsc_schur_pre[_schur_pre]);

      // set Schur AInv
      const std::string petsc_schur_ainv[] = {"diag", "lump"};
      po.inames.push_back(prefix + "mat_schur_complement_ainv_type");
      po.values.push_back(petsc_schur_ainv[_schur_ainv]);
    }

    // The DM associated with this split defines the subsplits' geometry.
    po.inames.push_back(dmprefix + "nfieldsplits");
    po.values.push_back(Moose::stringify(_splitting.size()));

    po.inames.push_back(dmprefix + "fieldsplit_names");
    po.values.push_back(Moose::stringify(_splitting));

    // Finally, recursively configure the splits contained within this split.
    for (const auto & split_name : _splitting)
    {
      std::shared_ptr<Split> split = _fe_problem.getNonlinearSystemBase().getSplit(split_name);
      std::string sprefix = prefix + "fieldsplit_" + split_name + "_";
      split->setup(sprefix);
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
    po.flags.push_back(prefix + op.substr(1));
  }
  // check if inames match values
  if (_petsc_options.values.size() != _petsc_options.inames.size())
    mooseError("PETSc option values do not match PETSc option names");

  for (std::size_t j = 0; j < _petsc_options.inames.size(); ++j)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = _petsc_options.inames[j];
    if (op[0] != '-')
      mooseError("Invalid PETSc option name ", op, " for Split ", _name);

    po.inames.push_back(prefix + op.substr(1));
    po.values.push_back(_petsc_options.values[j]);
  }
}
