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
#include "Split.h"
#include "InputParameters.h"
#include "PetscSupport.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

// petsc 3.3.0 or later needed
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)

template<>
InputParameters validParams<Split>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<std::vector<NonlinearVariableName> >("vars", "Variables Split operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<SubdomainName> >("blocks", "Mesh blocks Split operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<BoundaryName> >("sides", "Sidesets Split operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<BoundaryName> >("unsides", "Sidesets Split excludes (omitting this implies \"do not exclude any sidesets\"");
  params.addParam<std::vector<std::string> >("splitting", "The names of the splits (subsystems) in the decomposition of this split");

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
  params.addParam<MooseEnum>("schur_pre", SchurPreEnum, "Type of Schur complement preconditioner matrix");

  MooseEnum SchurAInvEnum("diag lump", "diag");
  params.addParam<MooseEnum>("schur_ainv", SchurAInvEnum, "Type of approximation to inv(A) used when forming S = D - C inv(A) B");

  params.addParam<MultiMooseEnum>("petsc_options", Moose::PetscSupport::getCommonPetscFlags(), "PETSc flags for the FieldSplit solver");
  params.addParam<MultiMooseEnum>("petsc_options_iname", Moose::PetscSupport::getCommonPetscKeys(), "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for the FieldSplit solver");

  params.registerBase("Split");
  return params;
}

Split::Split (const InputParameters & parameters) :
    MooseObject(parameters),
    Restartable(parameters, "Splits"),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblem *>("_fe_problem")),
    _vars(getParam<std::vector<NonlinearVariableName> >("vars")),
    _blocks(getParam<std::vector<SubdomainName> >("blocks")),
    _sides(getParam<std::vector<BoundaryName> >("sides")),
    _unsides(getParam<std::vector<BoundaryName> >("unsides")),
    _splitting(getParam<std::vector<std::string> >("splitting")),
    _splitting_type(getParam<MooseEnum>("splitting_type")),
    _schur_type(getParam<MooseEnum>("schur_type")),
    _schur_pre(getParam<MooseEnum>("schur_pre")),
    _schur_ainv(getParam<MooseEnum>("schur_ainv")),
    _petsc_options(_fe_problem.getPetscOptions())
{
}

void
Split::setup(const std::string& prefix)
{
  std::string dmprefix = prefix + "dm_moose_", opt, val;

  // var options
  if (_vars.size()) {
    opt = dmprefix + "vars";
    val="";
    for (unsigned int j = 0; j < _vars.size(); ++j)
      val += (j ? "," : "") + _vars[j];
    Moose::PetscSupport::setSinglePetscOption(opt, val);
  }

  // block options
  if (_blocks.size()) {
    opt = dmprefix + "blocks";
    val = "";
    for (unsigned int j = 0; j < _blocks.size(); ++j)
      val += (j ? "," : "") + _blocks[j];
    Moose::PetscSupport::setSinglePetscOption(opt, val);
  }

  // side options
  if (_sides.size()) {
    opt = dmprefix + "sides";
    val = "";
    for (unsigned int j = 0; j < _sides.size(); ++j)
      val += (j ? "," : "") + _sides[j];
    Moose::PetscSupport::setSinglePetscOption(opt, val);
  }

  // unside options
  if (_unsides.size()) {
    opt = dmprefix + "unsides";
    val = "";
    for (unsigned int j = 0; j < _unsides.size(); ++j)
      val += (j ? "," : "") + _unsides[j];
    Moose::PetscSupport::setSinglePetscOption(opt, val);
  }

  if (_splitting.size()) {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    opt = prefix + "pc_type";
    val = "fieldsplit";
    Moose::PetscSupport::setSinglePetscOption(opt, val);

    // set Splitting Type
    const char * petsc_splitting_type[] = {
      "additive",
      "multiplicative",
      "symmetric_multiplicative",
      "schur"
    };
    opt = prefix + "pc_fieldsplit_type";
    Moose::PetscSupport::setSinglePetscOption(opt, petsc_splitting_type[_splitting_type]);

    if (_splitting_type == SplittingTypeSchur)
    {
      // set Schur Type
      const char * petsc_schur_type[] = {
        "diag",
        "upper",
        "lower",
        "full"
      };
      opt = prefix + "pc_fieldsplit_schur_fact_type";
      Moose::PetscSupport::setSinglePetscOption(opt, petsc_schur_type[_splitting_type]);

      // set Schur Preconditioner
      const char * petsc_schur_pre[] = {
        "self",
        "selfp",
        #if PETSC_VERSION_LESS_THAN(3,4,0)
          "diag"
        #else
          "a11"
        #endif
      };
      opt = prefix + "pc_fieldsplit_schur_precondition";
      Moose::PetscSupport::setSinglePetscOption(opt, petsc_schur_pre[_schur_pre]);

      // set Schur AInv
      const char * petsc_schur_ainv[] = {
        "diag",
        "lump"
      };
      opt = prefix + "mat_schur_complement_ainv_type";
      Moose::PetscSupport::setSinglePetscOption(opt, petsc_schur_ainv[_schur_ainv]);
    }
    // FIXME: How would we support the user-provided Pmat?

    // The DM associated with this split defines the subsplits' geometry.
    opt = dmprefix + "nfieldsplits";
    std::ostringstream sval;
    sval << _splitting.size();
    val = sval.str();
    Moose::PetscSupport::setSinglePetscOption(opt, val);

    opt = dmprefix + "fieldsplit_names";
    val = "";
    for (unsigned int i = 0; i < _splitting.size(); ++i)
      val += (i ? "," : "") + _splitting[i];
    Moose::PetscSupport::setSinglePetscOption(opt, val);

    // Finally, recursively configure the splits contained within this split.
    for (unsigned int i = 0; i < _splitting.size(); ++i)
    {
      MooseSharedPointer<Split> split = _fe_problem.getNonlinearSystem().getSplit(_splitting[i]);
      std::string sprefix = prefix + "fieldsplit_" + _splitting[i] + "_";
      split->setup(sprefix);
    }
  }

  // Now we set the user-specified petsc options for this split, possibly overriding the above settings.
  for (unsigned j = 0; j < _petsc_options.flags.size(); ++j)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = _petsc_options.flags[j];
    if (op[0] != '-')
      mooseError("Invalid petsc option name " << op << " for Split " << _name);
    std::string opt = prefix + op.substr(1);
    Moose::PetscSupport::setSinglePetscOption(opt);
  }
  for (unsigned j = 0; j < _petsc_options.inames.size(); ++j)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = _petsc_options.inames[j];
    if (op[0] != '-')
      mooseError("Invalid petsc option name " << op << " for Split " << _name);
    std::string opt = prefix + op.substr(1);
    Moose::PetscSupport::setSinglePetscOption(opt, _petsc_options.values[j]);
  }
}

#else
// petsc earlier than 3.3.0.

void
Split::setup(const std::string& prefix)
{
}

#endif
