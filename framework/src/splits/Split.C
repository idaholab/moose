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
template<>
InputParameters validParams<Split>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::vector<std::string> >("vars",   "Variables Split operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<std::string> >("blocks", "Mesh blocks Split operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<std::string> >("sides",  "Sidesets Split operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<std::string> >("unsides",  "Sidesets Split excludes (omitting this implies \"do not exclude any sidesets\"");
  params.addParam<std::vector<std::string> >("splitting", "The names of the splits (subsystems) in the decomposition of this split");
  params.addParam<std::string>("splitting_type", "additive", "Split decomposition type: additive|multiplicative|symmetric_multiplicative|schur");
  params.addParam<std::string>("schur_type", "full", "Type of Schur complement: full|upper|lower");
  params.addParam<std::string>("schur_pre",  "S", "Type of Schur complement preconditioner matrix: S|Sp|A11");
  params.addParam<std::string>("schur_ainv",  "diag", "Type of approximation to inv(A) used when forming S = D - C inv(A) B: diag|lump");
  params.addParam<std::vector<std::string> >("petsc_options", "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for the FieldSplit solver");

  params.registerBase("Split");

  return params;
}

Split::Split (const std::string & name, InputParameters params) :
  MooseObject(name, params),
  Restartable(name, params, "Splits"),
  _fe_problem(*params.getCheckedPointerParam<FEProblem *>("_fe_problem")),
  _vars(getParam<std::vector<std::string> >("vars")),
  _blocks(getParam<std::vector<std::string> >("blocks")),
  _sides(getParam<std::vector<std::string> >("sides")),
  _unsides(getParam<std::vector<std::string> >("unsides")),
  _splitting(getParam<std::vector<std::string> >("splitting")),
  _splitting_type(getParam<std::string>("splitting_type")),
  _schur_type(getParam<std::string>("schur_type")),
  _schur_pre(getParam<std::string>("schur_pre")),
  _schur_ainv(getParam<std::string>("schur_ainv")),
  _petsc_options(getParam<std::vector<std::string> >("petsc_options")),
  _petsc_options_iname(getParam<std::vector<std::string> >("petsc_options_iname")),
  _petsc_options_value(getParam<std::vector<std::string> >("petsc_options_value"))
{}

void
Split::setup(const std::string& prefix)
{
  PetscErrorCode ierr;
  std::string    dmprefix = prefix+"dm_moose_", opt, val;

  // var options
  if (_vars.size()) {
    opt = dmprefix+"vars";
    val="";
    for (unsigned int j = 0; j < _vars.size(); ++j) {
      if (j) val += ",";
      val += _vars[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // block options
  if (_blocks.size()) {
    opt = dmprefix+"blocks";
    val="";
    for (unsigned int j = 0; j < _blocks.size(); ++j) {
      if (j) val += ",";
      val += _blocks[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // side options
  if (_sides.size()) {
    opt = dmprefix+"sides";
    val="";
    for (unsigned int j = 0; j < _sides.size(); ++j) {
      if (j) val += ",";
      val += _sides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // unside options
  if (_unsides.size()) {
    opt = dmprefix+"unsides";
    val="";
    for (unsigned int j = 0; j < _unsides.size(); ++j) {
      if (j) val += ",";
      val += _unsides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

  if (_splitting.size()) {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    opt = prefix+"pc_type";
    val = "fieldsplit";
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    SplittingType dtype = getSplittingType(_splitting_type,val);
    opt = prefix+"pc_fieldsplit_type";
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    if (dtype == SplittingTypeSchur) {
      getSchurType(_schur_type,val); // validation
      opt = prefix+"pc_fieldsplit_schur_fact_type";
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);

      getSchurPre(_schur_pre,val); // validation
      opt = prefix+"pc_fieldsplit_schur_precondition";
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);

      getSchurAinv(_schur_ainv,val);
      opt = prefix+"mat_schur_complement_ainv_type";
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);

    }
    // FIXME: How would we support the user-provided Pmat?

    // The DM associated with this split defines the subsplits' geometry.
    opt = dmprefix+"nfieldsplits";
    {std::ostringstream sval; sval << _splitting.size(); val = sval.str();}
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    opt = dmprefix+"fieldsplit_names";
    val = "";
    for (unsigned int i = 0; i < _splitting.size(); ++i) {
      if (i) val += ","; val += _splitting[i];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    // Finally, recursively configure the splits contained within this split.
    for (unsigned int i = 0; i < _splitting.size(); ++i) {
      std::string sname = _splitting[i];
      std::string sprefix = prefix+"fieldsplit_"+sname+"_";
      Split* split = _fe_problem.getNonlinearSystem().getSplit(sname);
      split->setup(sprefix);
    }
  }
  // Now we set the user-specified petsc options for this split, possibly overriding the above settings.
  for (unsigned j = 0; j < _petsc_options.size(); ++j) {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    std::string op = _petsc_options[j];
    if (op[0] != '-') {
      std::ostringstream err;
      err << "Invalid petsc option name " << op << " for Split " << _name;
      mooseError(err.str());
    }
    std::string opt = prefix+op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(),PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  for (unsigned j = 0; j < _petsc_options_iname.size(); ++j) {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    std::string op = _petsc_options_iname[j];
    if (op[0] != '-') {
      std::ostringstream err;
      err << "Invalid petsc option name " << op << " for Split " << _name;
      mooseError(err.str());
    }
    std::string opt = prefix+op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(),_petsc_options_value[j].c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
}

Split::SplittingType
Split::getSplittingType(const std::string& str, std::string& petsc_str)
{
  if (str=="additive") {
    petsc_str = "additive";
    return SplittingTypeAdditive;
  } else if (str=="multiplicative") {
    petsc_str = "multiplicative";
    return SplittingTypeMultiplicative;
  } else if (str=="symmetric_multiplicative") {
    petsc_str = "symmetric_multiplicative";
    return SplittingTypeSymmetricMultiplicative;
  } else if (str=="schur") {
    petsc_str = "schur";
    return SplittingTypeSchur;
  } else  mooseError(std::string("Invalid SplittingType: ") + str);
  return SplittingTypeAdditive;
}

Split::SchurType
Split::getSchurType(const std::string& str, std::string& petsc_str)
{
  if (str=="diagonal") {
    petsc_str = "diag";
    return SchurTypeDiag;
  } else if (str=="upper") {
    petsc_str = "upper";
    return SchurTypeUpper;
  } else if (str=="lower") {
    petsc_str = "lower";
    return SchurTypeLower;
  } else if (str=="full") {
    petsc_str = "full";
    return SchurTypeFull;
  } else  mooseError(std::string("Invalid SchurType: ") + str);
  return SchurTypeDiag;
}

Split::SchurPre
Split::getSchurPre(const std::string& str, std::string& petsc_str)
{
  if (str=="S") {
    petsc_str = "self";
    return SchurPreS;
  } else if (str=="Sp") {
    petsc_str = "selfp";
    return SchurPreSp;
  } else if (str=="A11") {
#if PETSC_VERSION_LESS_THAN(3,4,0)
    petsc_str = "diag";
#else
    petsc_str = "a11";
#endif
    return SchurPreA11;
  } else  mooseError(std::string("Invalid SchurPre: ") + str);
  return SchurPreA11;
}
Split::SchurAinv
Split::getSchurAinv(const std::string& str, std::string& petsc_str)
{
  if (str=="diag") {
    petsc_str = "diag";
    return SchurAdiag;
  } else if (str=="lump") {
    petsc_str = "lump";
    return SchurAlump;
  } else  mooseError(std::string("Invalid SchurAinv: ") + str);
  return SchurAdiag;
}
#else
/* petsc earlier than 3.3.0. */
void
Split::setup(const std::string& prefix)
{
}
#endif
