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
  params.addParam<std::vector<std::string> >("decomposition", "The names of the splits (subsystems) in the decomposition of this split");
  params.addParam<std::string>("decomposition_type", "additive", "Split decomposition type: additive|multiplicative|symmetric_multiplicative|schur");
  params.addParam<std::string>("schur_type", "full", "Type of Schur complement: full|upper|lower");
  params.addParam<std::string>("schur_pre",  "self", "Type of Schur complement preconditioner matrix: self|diag");
  params.addParam<std::vector<std::string> >("petsc_options", "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for the FieldSplit solver");

  return params;
}

Split::Split (const std::string & name, InputParameters params) :
  MooseObject(name, params),
  _fe_problem(*getParam<FEProblem*>("_fe_problem")),
  _vars(getParam<std::vector<std::string> >("vars")),
  _blocks(getParam<std::vector<std::string> >("blocks")),
  _sides(getParam<std::vector<std::string> >("sides")),
  _unsides(getParam<std::vector<std::string> >("unsides")),
  _decomposition(getParam<std::vector<std::string> >("decomposition")),
  _decomposition_type(getParam<std::string>("decomposition_type")),
  _schur_type(getParam<std::string>("schur_type")),
  _schur_pre(getParam<std::string>("schur_pre")),
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
      if(j) val += ",";
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
      if(j) val += ",";
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
      if(j) val += ",";
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
      if(j) val += ",";
      val += _unsides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

  if (_decomposition.size()) {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    opt = prefix+"pc_type";
    val = "fieldsplit";
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    DecompositionType dtype = getDecompositionType(_decomposition_type);
    opt = prefix+"pc_fieldsplit_type";
    val = _decomposition_type;
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    if (dtype == DecompositionTypeSchur) {
      SchurType stype = getSchurType(_schur_type); // validation
      opt = prefix+"pc_fieldsplit_schur_fact_type";
      val = _schur_type;
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);

      SchurPreconditioner sptype = getSchurPreconditioner(_schur_pre); // validation
      opt = prefix+"pc_fieldsplit_schur_precondition";
      val = _schur_pre;
      ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
      CHKERRABORT(libMesh::COMM_WORLD,ierr);
    }
    // FIXME: How would we support the user-provided Pmat?

    // The DM associated with this split defines the subsplits' geometry.
    opt = dmprefix+"nfieldsplits";
    {std::ostringstream sval; sval << _decomposition.size(); val = sval.str();}
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    opt = dmprefix+"fieldsplit_names";
    val = "";
    for (unsigned int i = 0; i < _decomposition.size(); ++i) {
      if (i) val += ","; val += _decomposition[i];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    // Finally, recursively configure the splits contained within this split.
    for (unsigned int i = 0; i < _decomposition.size(); ++i) {
      std::string sname = _decomposition[i];
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

Split::DecompositionType
Split::getDecompositionType(const std::string& str)
{
  if(str=="additive")                      return DecompositionTypeAdditive;
  else if(str=="multiplicative")           return DecompositionTypeMultiplicative;
  else if(str=="symmetric_multiplicative") return DecompositionTypeSymmetricMultiplicative;
  else if(str=="schur")                    return DecompositionTypeSchur;
  else  mooseError(std::string("Invalid DecompositionType: ") + str);
  return DecompositionTypeAdditive;
}

Split::SchurType
Split::getSchurType(const std::string& str)
{
  if(str=="diagonal")            return SchurTypeDiag;
  else if(str=="upper")          return SchurTypeUpper;
  else if(str=="lower")          return SchurTypeLower;
  else if(str=="full")           return SchurTypeFull;
  else  mooseError(std::string("Invalid SchurType: ") + str);
  return SchurTypeDiag;
}

Split::SchurPreconditioner
Split::getSchurPreconditioner(const std::string& str)
{
  if(str=="self")            return SchurPreconditionerSelf;
  else if(str=="diag")          return SchurPreconditionerD;
  else  mooseError(std::string("Invalid SchurPreconditioner: ") + str);
  return SchurPreconditionerD;
}
#else
void
Split::setup(){}
#endif


