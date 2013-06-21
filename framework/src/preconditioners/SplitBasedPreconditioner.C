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
#include "SplitBasedPreconditioner.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "PetscSupport.h"
#include "MooseEnum.h"

//libMesh Includes
#include "libmesh/libmesh_common.h"
#include "libmesh/petsc_nonlinear_solver.h"

template<>
InputParameters validParams<SplitBasedPreconditioner>()
{
  InputParameters params = validParams<MoosePreconditioner>();

  params.addParam<std::vector<std::string> >("off_diag_row", "The off diagonal row you want to add into the matrix, it will be associated with an off diagonal column from the same position in off_diag_colum.");
  params.addParam<std::vector<std::string> >("off_diag_column", "The off diagonal column you want to add into the matrix, it will be associated with an off diagonal row from the same position in off_diag_row.");
  params.addParam<bool>("full", false, "Set to true if you want the full set of couplings.  Simply for convenience so you don't have to set every off_diag_row and off_diag_column combination.");
  return params;
}

SplitBasedPreconditioner::SplitBasedPreconditioner (const std::string & name, InputParameters params) :
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
  _nl.useSplitBasedPreconditioner(true);
}

void
SplitBasedPreconditioner::setup()
{
}
#endif
