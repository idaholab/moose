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

  params.addParam<std::string>("split_type", "additive", "Type of FieldSplit preconditioner to use: additive|multiplicative|symmetric_multiplicative|schur");
  params.addParam<std::string>("schur_type", "full", "Style of FieldSplit Schur factorization to use: upper|lower|full");
  params.addParam<std::string>("schur_preconditioner", "diag", "Preconditioning matrix to use with Schur: self|diag");
  params.addParam<std::vector<std::string> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<std::string> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");
  params.addParam<bool>("full", false, "Set to true if you want the full set of couplings.  Simply for convenience so you don't have to set every off_diag_row and off_diag_column combination.");
  params.addParam<std::vector<std::string> >("split_names", "The names of the splits");

  return params;
}

FieldSplitPreconditioner::FieldSplitPreconditioner (const std::string & name, InputParameters params) :
    MoosePreconditioner(name, params),
    _nl(_fe_problem.getNonlinearSystem()),
    _split_type(getSplitType(getParam<std::string>("split_type"))),
    _schur_type(getSchurType(getParam<std::string>("schur_type"))),
    _schur_preconditioner(getSchurPreconditioner(getParam<std::string>("schur_preconditioner")))
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

  _nl.setPreconditioner(this);
  _nl.useFieldSplitPreconditioner(true);
}

void
FieldSplitPreconditioner::setup()
{
  static bool     DMMooseRegistered = false;
  PetscErrorCode  ierr;

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
  // const std::vector<std::string>& splitvec =  getParam<std::vector<std::string> >("split_names");
  // std::set<std::string> splitset;
  // for (unsigned int i = 0; i < splitvec.size(); ++i) splitset.insert(splitvec[i]);
  // ierr = DMMooseSetSplitNames(dm,splitset);
  // CHKERRABORT(libMesh::COMM_WORLD,ierr);
  // for (std::set<std::string>::const_iterator spit = splitset.begin(); spit != splitset.end(); ++spit) {
  //   std::string spname = *spit;
  //   {
  //     std::string pname = spname+"_vars";
  //     //parameters().addParam<std::vector<std::string> >(pname,std::vector<std::string>,"Split variables");
  //     const std::vector<std::string>& varvec = getParam<std::vector<std::string> >(pname);
  //     std::set<std::string> varset;
  //     for (unsigned int i = 0; i < varvec.size(); ++i) varset.insert(varvec[i]);
  //     if (varset.size()) {
  // 	ierr = DMMooseSetSplitVars(dm,spname,varset);
  // 	CHKERRABORT(libMesh::COMM_WORLD,ierr);
  //     }
  //   }
  //   {
  //     std::string pname = spname+"_blocks";
  //     const std::vector<std::string>& blockvec = getParam<std::vector<std::string> >(pname);
  //     std::set<std::string> blockset;
  //     for (unsigned int i = 0; i < blockvec.size(); ++i) blockset.insert(blockvec[i]);
  //     if(blockset.size()) {
  // 	ierr = DMMooseSetSplitBlocks(dm,spname,blockset);
  // 	CHKERRABORT(libMesh::COMM_WORLD,ierr);
  //     }
  //   }
  //   {
  //     std::string pname = spname+"_sides";
  //     const std::vector<std::string>& sidevec = getParam<std::vector<std::string> >(pname);
  //     std::set<std::string> sideset;
  //     for (unsigned int i = 0; i < sidevec.size(); ++i) sideset.insert(sidevec[i]);
  //     if(sideset.size()) {
  // 	ierr = DMMooseSetSplitSides(dm,spname,sideset);
  // 	CHKERRABORT(libMesh::COMM_WORLD,ierr);
  //     }
  //   }
  // }
  ierr = DMSetFromOptions(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = DMSetUp(dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = SNESSetDM(snes,dm);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

  /* Translate Moose options into PC options. */
  KSP ksp;
  PC  pc;
  ierr = SNESGetKSP(snes, &ksp);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  ierr = KSPGetPC(ksp, &pc);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  /* PC type. */
  ierr = PCSetType(pc,PCFIELDSPLIT);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  /* PCFieldSplit type. */
  PCCompositeType fstype;
  switch(_split_type) {
  case SplitTypeAdditive:
    fstype = PC_COMPOSITE_ADDITIVE;
    break;
  case SplitTypeMultiplicative:
    fstype = PC_COMPOSITE_MULTIPLICATIVE;
    break;
  case SplitTypeSymmetricMultiplicative:
    fstype = PC_COMPOSITE_SYMMETRIC_MULTIPLICATIVE;
    break;
  case SplitTypeSchur:
    fstype = PC_COMPOSITE_SCHUR;
    break;
  default:
    std::ostringstream err;
    err << "Unknown FieldSplitPreconditioner split type: " << _split_type;
    mooseError(err.str());
    break;
  }
  ierr = PCFieldSplitSetType(pc,fstype);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);
  /* PCFieldSplitSchurFactType. */
  PCFieldSplitSchurFactType sftype;
  switch(_schur_type) {
  case SchurTypeDiag:
    sftype = PC_FIELDSPLIT_SCHUR_FACT_DIAG;
    break;
  case SchurTypeUpper:
    sftype = PC_FIELDSPLIT_SCHUR_FACT_UPPER;
    break;
  case SchurTypeLower:
    sftype = PC_FIELDSPLIT_SCHUR_FACT_LOWER;
    break;
  case SchurTypeFull:
    sftype = PC_FIELDSPLIT_SCHUR_FACT_FULL;
    break;
  default:
    std::ostringstream err;
    err << "Unknown FieldSplitPreconditioner Schur type: " << _schur_type;
    mooseError(err.str());
    break;
  }
  ierr = PCFieldSplitSetSchurFactType(pc,sftype);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

    /* PCFieldSplitSchurFactType. */
  PCFieldSplitSchurPreType sptype;
  switch(_schur_preconditioner) {
  case SchurPreconditionerSelf:
    sptype = PC_FIELDSPLIT_SCHUR_PRE_SELF;
    break;
  case SchurPreconditionerD:
    sptype = PC_FIELDSPLIT_SCHUR_PRE_DIAG;
    break;
  default:
    std::ostringstream err;
    err << "Unknown FieldSplitPreconditioner SchurPreconditioner: " << _schur_preconditioner;
    mooseError(err.str());
    break;
  }
  // FIXME: How would we support the user-provided Pmat?
  ierr = PCFieldSplitSchurPrecondition(pc,sptype,PETSC_NULL);
  CHKERRABORT(libMesh::COMM_WORLD,ierr);

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


