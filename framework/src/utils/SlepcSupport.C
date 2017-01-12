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

#ifndef SLEPCSUPPORT_H
#define SLEPCSUPPORT_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_SLEPC

#include "SlepcSupport.h"
// MOOSE includes
#include "MultiMooseEnum.h"
#include "InputParameters.h"
#include "EigenProblem.h"
#include "Conversion.h"

namespace Moose
{
namespace SlepcSupport
{

  InputParameters
  getSlepcValidParams()
  {
    InputParameters params = emptyInputParameters();

    // eigen solve type
    MooseEnum eigen_solve_type("POWER ARNOLDI KRYLOVSCHUR JD");
    params.addParam<MooseEnum>("eigen_solve_type", eigen_solve_type,
                               "POWER: Power / Inverse / RQI "
                               "ARNOLDI: Arnoldi "
                               "KRYLOVSCHUR: Krylov-Schur "
                               "JD: Jacobi-Davidson ");
   return params;
  }

  InputParameters
  getSlepcEigenProblemValidParams()
  {
    InputParameters params = emptyInputParameters();

    // We are solving a Non-Hermitian eigenvalue problem by default
    MooseEnum eigen_problem_type("HEP NHEP GHEP GNHEP GHIEP");
    params.addParam<MooseEnum>("eigen_problem_type", eigen_problem_type,
                               "HEP: Hermitian "
                               "NHEP: Non-Hermitian "
                               "GHEP: Gerneralized Hermitian "
                               "GNHEP: Generalized Non-Hermitian "
                               "GHIEP: Generalized indefinite Hermitian ");
   return params;
  }

  void
  storeSlepcEigenProblemOptions(EigenProblem & eigen_problem, const InputParameters & params)
  {
    // eigen problem type
    if (params.isParamValid("eigen_problem_type"))
    {
      // Extract the eigen problem type
      const std::string & eigen_problem_type = params.get<MooseEnum>("eigen_problem_type");
      eigen_problem.solverParams()._eigen_problem_type = Moose::stringToEnum<Moose::EigenProblemType>(eigen_problem_type);
    }
  }

  void
  storeSlepcOptions(FEProblemBase & fe_problem, const InputParameters & params)
  {
    // if it is not an eigenvalue  problem, do nothing and return.
    if (!(dynamic_cast<EigenProblem *>(&fe_problem)))
      return;

    // can not use solve_type for eigenvalue problems
    if (params.isParamValid("solve_type"))
    {
      mooseError("Can not use solve_type for eigenvalue problems, please use eigen_solve_type instead \n");
    }
    // eigen solve type
    if (params.isParamValid("eigen_solve_type"))
    {
      // Extract the eigen solve type
      const std::string & eigen_solve_type = params.get<MooseEnum>("eigen_solve_type");
      fe_problem.solverParams()._eigen_solve_type = Moose::stringToEnum<Moose::EigenSolveType>(eigen_solve_type);
    }
    // eigen solve type
    if (params.isParamValid("eigen_problem_type"))
    {
      // Extract the eigen problem type
      const std::string & eigen_problem_type = params.get<MooseEnum>("eigen_problem_type");
      fe_problem.solverParams()._eigen_problem_type = Moose::stringToEnum<Moose::EigenProblemType>(eigen_problem_type);
    }
  }

  void
  setEigenProblemOptions(SolverParams & solver_params)
  {
    // set eigen problem types
    switch (solver_params._eigen_problem_type)
    {
    case Moose::EPT_HEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_hermitian");
      break;

    case Moose::EPT_NHEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_non_hermitian");
      break;

    case Moose::EPT_GHEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_hermitian");
      break;

    case Moose::EPT_GHIEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_indefinite");
      break;

    case Moose::EPT_GNHEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_gen_non_hermitian");
      break;

    case Moose::EPT_PGNHEP:
      Moose::PetscSupport::setSinglePetscOption("-eps_pos_gen_non_hermitian");
      break;

    default:
      mooseError("Unknown eigen solver type \n");
    }
  }

  void
  setEigenSolverOptions(SolverParams & solver_params)
  {
    // set SLEPC solver options
    switch (solver_params._eigen_solve_type)
    {
    case Moose::EST_POWER:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "power");
      break;

    case Moose::EST_ARNOLDI:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "arnoldi");
      break;

    case Moose::EST_KRYLOVSCHUR:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "krylovschur");
      break;

    case Moose::EST_JD:
      Moose::PetscSupport::setSinglePetscOption("-eps_type", "jd");
      break;

    default:
      mooseError("Unknown eigen solver type \n");
    }
  }

  void
  slepcSetOptions(FEProblemBase & problem)
  {
    Moose::PetscSupport::petscSetOptions(problem);
    setEigenSolverOptions(problem.solverParams());
    setEigenProblemOptions(problem.solverParams());
    Moose::PetscSupport::addPetscOptionsFromCommandline();
  }

} // namespace SlepcSupport
} // namespace moose

#endif // LIBMESH_HAVE_SLEPC

#endif // SLEPCSUPPORT_H
