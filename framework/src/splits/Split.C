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

#include "Split.h"
#include "InputParameters.h"

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
// petsc 3.3.0 or later needed

template<>
InputParameters validParams<Split>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<NonlinearVariableName> >("vars", "Variables Split operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<std::string> >("blocks", "Mesh blocks Split operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<std::string> >("sides", "Sidesets Split operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<std::string> >("unsides", "Sidesets Split excludes (omitting this implies \"do not exclude any sidesets\"");
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

  params.addParam<std::vector<std::string> >("petsc_options", "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for the FieldSplit solver");

  params.registerBase("Split");
  return params;
}

Split::Split(const std::string & name, InputParameters params) :
    MooseObject(name, params),
    Restartable(params, "Splits"),
    _fe_problem(*params.getCheckedPointerParam<FEProblem *>("_fe_problem")),
    _vars(getParam<std::vector<NonlinearVariableName> >("vars")),
    _blocks(getParam<std::vector<std::string> >("blocks")),
    _sides(getParam<std::vector<std::string> >("sides")),
    _unsides(getParam<std::vector<std::string> >("unsides")),
    _splitting(getParam<std::vector<std::string> >("splitting")),
    _splitting_type(getParam<MooseEnum>("splitting_type")),
    _schur_type(getParam<MooseEnum>("schur_type")),
    _schur_pre(getParam<MooseEnum>("schur_pre")),
    _schur_ainv(getParam<MooseEnum>("schur_ainv")),
    _petsc_options(getParam<std::vector<std::string> >("petsc_options")),
    _petsc_options_iname(getParam<std::vector<std::string> >("petsc_options_iname")),
    _petsc_options_value(getParam<std::vector<std::string> >("petsc_options_value"))
{
}

void
Split::setup(const std::string& prefix)
{
  PetscErrorCode ierr;
  std::string dmprefix = prefix + "dm_moose_", opt, val;

  // var options
  if (_vars.size()) {
    opt = dmprefix + "vars";
    val="";
    for (unsigned int j = 0; j < _vars.size(); ++j) {
      if (j) val += ",";
      val += _vars[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);
  }

  // block options
  if (_blocks.size()) {
    opt = dmprefix + "blocks";
    val = "";
    for (unsigned int j = 0; j < _blocks.size(); ++j) {
      if (j) val += ",";
      val += _blocks[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);
  }

  // side options
  if (_sides.size()) {
    opt = dmprefix + "sides";
    val = "";
    for (unsigned int j = 0; j < _sides.size(); ++j) {
      if (j) val += ",";
      val += _sides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);
  }

  // unside options
  if (_unsides.size()) {
    opt = dmprefix + "unsides";
    val = "";
    for (unsigned int j = 0; j < _unsides.size(); ++j) {
      if (j) val += ",";
      val += _unsides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);
  }

  if (_splitting.size()) {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    opt = prefix + "pc_type";
    val = "fieldsplit";
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);

    // set Splitting Type
    const char * petsc_splitting_type[] = {
      "additive",
      "multiplicative",
      "symmetric_multiplicative",
      "schur"
    };
    opt = prefix + "pc_fieldsplit_type";
    ierr = PetscOptionsSetValue(opt.c_str(), petsc_splitting_type[_splitting_type]);
    CHKERRABORT(_communicator.get(), ierr);

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
      ierr = PetscOptionsSetValue(opt.c_str(), petsc_schur_type[_splitting_type]);
      CHKERRABORT(_communicator.get(), ierr);

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
      ierr = PetscOptionsSetValue(opt.c_str(), petsc_schur_pre[_schur_pre]);
      CHKERRABORT(_communicator.get(), ierr);

      // set Schur AInv
      const char * petsc_schur_ainv[] = {
        "diag",
        "lump"
      };
      opt = prefix + "mat_schur_complement_ainv_type";
      ierr = PetscOptionsSetValue(opt.c_str(), petsc_schur_ainv[_schur_ainv]);
      CHKERRABORT(_communicator.get(), ierr);
    }
    // FIXME: How would we support the user-provided Pmat?

    // The DM associated with this split defines the subsplits' geometry.
    opt = dmprefix + "nfieldsplits";
    std::ostringstream sval;
    sval << _splitting.size();
    val = sval.str();
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);

    opt = dmprefix + "fieldsplit_names";
    val = "";
    for (unsigned int i = 0; i < _splitting.size(); ++i)
    {
      if (i) val += ",";
      val += _splitting[i];
    }
    ierr = PetscOptionsSetValue(opt.c_str(), val.c_str());
    CHKERRABORT(_communicator.get(), ierr);

    // Finally, recursively configure the splits contained within this split.
    for (unsigned int i = 0; i < _splitting.size(); ++i)
    {
      Split* split = _fe_problem.getNonlinearSystem().getSplit(_splitting[i]);
      std::string sprefix = prefix + "fieldsplit_" + _splitting[i] + "_";
      split->setup(sprefix);
    }
  }

  // Now we set the user-specified petsc options for this split, possibly overriding the above settings.
  for (unsigned j = 0; j < _petsc_options.size(); ++j)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = _petsc_options[j];
    if (op[0] != '-')
      mooseError("Invalid petsc option name " << op << " for Split " << _name);
    std::string opt = prefix + op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(), PETSC_NULL);
    CHKERRABORT(_communicator.get(), ierr);
  }
  for (unsigned j = 0; j < _petsc_options_iname.size(); ++j)
  {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    const std::string & op = _petsc_options_iname[j];
    if (op[0] != '-')
      mooseError("Invalid petsc option name " << op << " for Split " << _name);
    std::string opt = prefix + op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(), _petsc_options_value[j].c_str());
    CHKERRABORT(_communicator.get(), ierr);
  }
}

#else
// petsc earlier than 3.3.0.

void
Split::setup(const std::string& prefix)
{
}

#endif
