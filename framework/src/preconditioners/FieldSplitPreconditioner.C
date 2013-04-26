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

#include "libmesh/petsc_macro.h"
#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
#include "FieldSplitPreconditioner.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "MooseEnum.h"

//libMesh Includes
#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"

#include <PetscDMMoose.h>
#include <petscsnes.h>
EXTERN_C_BEGIN
extern PetscErrorCode DMCreate_Moose(DM);
EXTERN_C_END

template<>
InputParameters validParams<FieldSplitPreconditioner>()
{
  InputParameters params = validParams<MoosePreconditioner>();

  params.addParam<std::vector<std::string> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<std::string> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");
  params.addParam<bool>("full", false, "Set to true if you want the full set of couplings.  Simply for convenience so you don't have to set every off_diag_row and off_diag_column combination.");
  params.addParam<std::vector<std::string> >("vars",   "Variables FieldSplit decomposition operates on (omitting this implies \"all variables\"");
  params.addParam<std::vector<std::string> >("blocks", "Mesh blocks FieldSplit decomposition operates on (omitting this implies \"all blocks\"");
  params.addParam<std::vector<std::string> >("sides",  "Sidesets FieldSplit decomposition operates on (omitting this implies \"no sidesets\"");
  params.addParam<std::vector<std::string> >("splits", "The names of the FieldSplit splits (subsystems)");
  params.addParam<std::string>("fieldsplit_type", "additive", "FieldSplit decomposition type: additive|multiplicative|symmetric_multiplicative|schur");
  params.addParam<std::string>("schur_type", "full", "Type of Schur complement: full|upper|lower");
  params.addParam<std::string>("schur_pre",  "self", "Type of Schur complement preconditioner matrix: self|diag");
  params.addParam<std::vector<std::string> >("petsc_options", "PETSc flags for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_iname", "PETSc option names for the FieldSplit solver");
  params.addParam<std::vector<std::string> >("petsc_options_value", "PETSc option values for the FieldSplit solver");

  return params;
}

FieldSplitPreconditioner::FieldSplitPreconditioner (const std::string & name, InputParameters params) :
    MoosePreconditioner(name, params),
    _nl(_fe_problem.getNonlinearSystem())
{
  unsigned int n_vars        = _nl.nVariables();
  bool full = getParam<bool>("full");
  CouplingMatrix *cm = new CouplingMatrix(n_vars);
  if (!full)
  {
    // put 1s on diagonal
    for (unsigned int i = 0; i < n_vars; i++)
      (*cm)(i, i) = 1;

    // off-diagonal entries
    std::vector<std::vector<unsigned int> > off_diag(n_vars);
    for(unsigned int i = 0; i < getParam<std::vector<std::string> >("off_diag_row").size(); i++)
    {
      unsigned int row = _nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_row")[i]).index();
      unsigned int column = _nl.getVariable(0, getParam<std::vector<std::string> >("off_diag_column")[i]).index();
      (*cm)(row, column) = 1;
    }
  }
  else
  {
    for (unsigned int i = 0; i < n_vars; i++)
      for (unsigned int j = 0; j < n_vars; j++)
        (*cm)(i,j) = 1;
  }
  _fe_problem.setCouplingMatrix(cm);
  _nl.useFieldSplitPreconditioner(true);
  // Create a special top-level split with an empty string for the name.
  NonlinearSystem::FieldSplitInfo info;
  info.name   = "";
  info.vars   = getParam<std::vector<std::string> >("vars");
  info.blocks = getParam<std::vector<std::string> >("blocks");
  info.sides  = getParam<std::vector<std::string> >("sides");
  info.splits = getParam<std::vector<std::string> >("splits");
  info.fieldsplit_type = getParam<std::string>("fieldsplit_type");
  info.schur_type = getParam<std::string>("schur_type");
  info.schur_pre  = getParam<std::string>("schur_pre");
  info.petsc_options = getParam<std::vector<std::string> >("petsc_options");
  info.petsc_options_iname = getParam<std::vector<std::string> >("petsc_options_iname");
  info.petsc_options_value = getParam<std::vector<std::string> >("petsc_options_value");
  _nl.addFieldSplit(info.name, info);
}

void
FieldSplitPreconditioner::setup()
{
  static bool     DMMooseRegistered = false;
  PetscErrorCode  ierr;


  // Set up the top-level split, which will recursively set up the subsplits
  setupSplit("");

  // Create and set up the DM that will consume the split options set above.
  if (!DMMooseRegistered) {
    ierr = DMRegister(DMMOOSE, PETSC_NULL, "DMCreate_Moose", DMCreate_Moose);
    CHKERRABORT(libMesh::COMM_WORLD, ierr);
    DMMooseRegistered = true;
  }

  PetscNonlinearSolver<Number> *petsc_solver = dynamic_cast<PetscNonlinearSolver<Number> *>(_nl.sys().nonlinear_solver.get());
  SNES snes = petsc_solver->snes();
  DM dm;
  ierr = DMCreateMoose(libMesh::COMM_WORLD, _nl, &dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMSetFromOptions(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMSetUp(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = SNESSetDM(snes,dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMDestroy(&dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
}

void FieldSplitPreconditioner::setupSplit(const std::string& name, const std::string& prefix)
{
  PetscErrorCode ierr;
  std::string    dmprefix = prefix+"dm_moose_", opt, val;
  const NonlinearSystem::FieldSplitInfo& info = _nl.getFieldSplit(name);

  // var options
  if (info.vars.size()) {
    opt = dmprefix+"vars";
    val="";
    for (unsigned int j = 0; j < info.vars.size(); ++j) {
      if(j) val += ",";
      val += info.vars[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // block options
  if (info.blocks.size()) {
    opt = dmprefix+"blocks";
    val="";
    for (unsigned int j = 0; j < info.blocks.size(); ++j) {
      if(j) val += ",";
      val += info.blocks[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  // side options
  if (info.sides.size()) {
    opt = dmprefix+"sides";
    val="";
    for (unsigned int j = 0; j < info.blocks.size(); ++j) {
      if(j) val += ",";
      val += info.sides[j];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }

  if (info.splits.size()) {
    // If this split has subsplits, it is presumed that the pc_type used to solve this split's subsystem is fieldsplit
    // with the following parameters (unless overridden by the user-specified petsc_options below).
    opt = prefix+"pc_type";
    val = "fieldsplit";
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    //SplitType fstype = getSplitType(info.type); // validation
    opt = prefix+"pc_fieldsplit_type";
    val = info.fieldsplit_type;
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    //SchurType stype = getSchurType(info.schur_type); // validation
    opt = prefix+"pc_fieldsplit_schur_fact_type";
    val = info.schur_type;
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    //SchurPreconditioner sptype = getSchurPreconditioner(info.schur_pre); // validation
    opt = prefix+"pc_fieldsplit_schur_precondition";
    val = info.schur_pre;
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    // FIXME: How would we support the user-provided Pmat?

    // The DM associated with this split defines the subsplits' geometry.
    opt = dmprefix+"nfieldsplits";
    {std::ostringstream sval; sval << info.splits.size(); val = sval.str();}
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
    opt = dmprefix+"fieldsplit_names";
    val = "";
    for (unsigned int i = 0; i < info.splits.size(); ++i) {
      if (i) val += ","; val += info.splits[i];
    }
    ierr = PetscOptionsSetValue(opt.c_str(),val.c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);

    // Finally, recursively configure the splits contained within this split.
    for (unsigned int i = 0; i < info.splits.size(); ++i) {
      std::string sname = info.splits[i];
      std::string sprefix = prefix+"fieldsplit_"+sname+"_";
      setupSplit(sname, sprefix);
    }
  }
  // Now we set the user-specified petsc options for this split, possibly overriding the above settings.
  for (unsigned j = 0; j < info.petsc_options.size(); ++j) {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    std::string op = info.petsc_options[j];
    if (op[0] != '-') {
      std::ostringstream err;
      err << "Invalid petsc option name " << op << " for FieldSplit " << info.name;
      mooseError(err.str());
    }
    std::string opt = prefix+op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(),PETSC_NULL);
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
  for (unsigned j = 0; j < info.petsc_options_iname.size(); ++j) {
    // Need to prepend the prefix and strip off the leading '-' on the option name.
    std::string op = info.petsc_options_iname[j];
    if (op[0] != '-') {
      std::ostringstream err;
      err << "Invalid petsc option name " << op << " for FieldSplit " << info.name;
      mooseError(err.str());
    }
    std::string opt = prefix+op.substr(1);
    ierr = PetscOptionsSetValue(opt.c_str(),info.petsc_options_value[j].c_str());
    CHKERRABORT(libMesh::COMM_WORLD,ierr);
  }
}

FieldSplitPreconditioner::~FieldSplitPreconditioner () {}

FieldSplitPreconditioner::SplitType
FieldSplitPreconditioner::getSplitType(const std::string& str)
{
  if(str=="additive")                      return SplitTypeAdditive;
  else if(str=="multiplicative")           return SplitTypeMultiplicative;
  else if(str=="symmetric_multiplicative") return SplitTypeSymmetricMultiplicative;
  else if(str=="schur")                    return SplitTypeSchur;
  else  mooseError(std::string("Invalid FieldSplitPreconditioner style: ") + str);
  return SplitTypeAdditive;
}

FieldSplitPreconditioner::SchurType
FieldSplitPreconditioner::getSchurType(const std::string& str)
{
  if(str=="diagonal")            return SchurTypeDiag;
  else if(str=="upper")          return SchurTypeUpper;
  else if(str=="lower")          return SchurTypeLower;
  else if(str=="full")           return SchurTypeFull;
  else  mooseError(std::string("Invalid FieldSplitPreconditioner Schur style: ") + str);
  return SchurTypeDiag;
}

FieldSplitPreconditioner::SchurPreconditioner
FieldSplitPreconditioner::getSchurPreconditioner(const std::string& str)
{
  if(str=="self")            return SchurPreconditionerSelf;
  else if(str=="diag")          return SchurPreconditionerD;
  else  mooseError(std::string("Invalid FieldSplitPreconditioner SchurPreconditioner: ") + str);
  return SchurPreconditionerD;
}
#endif


