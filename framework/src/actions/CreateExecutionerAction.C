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

#include "CreateExecutionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "Conversion.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"

template<>
InputParameters validParams<CreateExecutionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addParam<Real>        ("l_tol",           1.0e-5,   "Linear Tolerance");
  params.addParam<Real>        ("l_abs_step_tol",  -1,       "Linear Absolute Step Tolerance");
  params.addParam<unsigned int>("l_max_its",       10000,    "Max Linear Iterations");
  params.addParam<unsigned int>("nl_max_its",      50,       "Max Nonlinear Iterations");
  params.addParam<unsigned int>("nl_max_funcs",    10000,    "Max Nonlinear solver function evaluations");
  params.addParam<Real>        ("nl_abs_tol",      1.0e-50,  "Nonlinear Absolute Tolerance");
  params.addParam<Real>        ("nl_rel_tol",      1.0e-8,   "Nonlinear Relative Tolerance");
  params.addParam<Real>        ("nl_abs_step_tol", 1.0e-50,  "Nonlinear Absolute step Tolerance");
  params.addParam<Real>        ("nl_rel_step_tol", 1.0e-50,  "Nonlinear Relative step Tolerance");
  params.addParam<bool>        ("no_fe_reinit",    false,    "Specifies whether or not to reinitialize FEs");

  params.addParamNamesToGroup("l_tol l_abs_step_tol l_max_its nl_max_its nl_max_funcs nl_abs_tol nl_rel_tol nl_abs_step_tol nl_rel_step_tol", "Solver");
  params.addParamNamesToGroup("no_fe_reinit", "Advanced");

  return params;
}

void
CreateExecutionerAction::populateCommonExecutionerParams(InputParameters & params)
{
}

CreateExecutionerAction::CreateExecutionerAction(InputParameters params) :
    MooseObjectAction(params)
{
}

void
CreateExecutionerAction::act()
{
  // Steady and derived Executioners need to know the number of adaptivity steps to take.  This parameter
  // is held in the child block Adaptivity and needs to be pulled early
  if (_problem.get() != NULL)
  {
    // Extract and store PETSc related settings on FEProblem
#ifdef LIBMESH_HAVE_PETSC
    storePetscOptions(*_problem, _moose_object_pars);
#endif //LIBMESH_HAVE_PETSC

    // solver params
    EquationSystems & es = _problem->es();
    es.parameters.set<Real> ("linear solver tolerance")
      = getParam<Real>("l_tol");

    es.parameters.set<Real> ("linear solver absolute step tolerance")
      = getParam<Real>("l_abs_step_tol");

    es.parameters.set<unsigned int> ("linear solver maximum iterations")
      = getParam<unsigned int>("l_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum iterations")
      = getParam<unsigned int>("nl_max_its");

    es.parameters.set<unsigned int> ("nonlinear solver maximum function evaluations")
      = getParam<unsigned int>("nl_max_funcs");

    es.parameters.set<Real> ("nonlinear solver absolute residual tolerance")
      = getParam<Real>("nl_abs_tol");

    es.parameters.set<Real> ("nonlinear solver relative residual tolerance")
      = getParam<Real>("nl_rel_tol");

    es.parameters.set<Real> ("nonlinear solver absolute step tolerance")
      = getParam<Real>("nl_abs_step_tol");

    es.parameters.set<Real> ("nonlinear solver relative step tolerance")
      = getParam<Real>("nl_rel_step_tol");

#ifdef LIBMESH_HAVE_PETSC
    _problem->getNonlinearSystem()._l_abs_step_tol = getParam<Real>("l_abs_step_tol");
#endif

  }

  Moose::setup_perf_log.push("Create Executioner","Setup");
  _moose_object_pars.set<FEProblem *>("_fe_problem") = _problem.get();
  MooseSharedPointer<Executioner> executioner = MooseSharedNamespace::static_pointer_cast<Executioner>(_factory.create(_type, "Executioner", _moose_object_pars));
  Moose::setup_perf_log.pop("Create Executioner","Setup");

  _awh.executioner() = executioner;
}

#ifdef LIBMESH_HAVE_PETSC
void
CreateExecutionerAction::storePetscOptions(FEProblem & fe_problem, InputParameters & params)
{
  // Note: Options set in the Preconditioner block will override those set in the Executioner block
  if (params.isParamValid("solve_type"))
  {
    // Extract the solve type
    const std::string & solve_type = params.get<MooseEnum>("solve_type");
    fe_problem.solverParams()._type = Moose::stringToEnum<Moose::SolveType>(solve_type);
  }

  if (params.isParamValid("line_search"))
  {
      MooseEnum line_search = params.get<MooseEnum>("line_search");
      if (fe_problem.solverParams()._line_search == Moose::LS_INVALID || line_search != "default")
        fe_problem.solverParams()._line_search = Moose::stringToEnum<Moose::LineSearchType>(line_search);
  }

  // The parameters contained in the Action
  const MultiMooseEnum           & petsc_options        = params.get<MultiMooseEnum>("petsc_options");
  const std::vector<std::string> & petsc_options_inames = params.get<std::vector<std::string> >("petsc_options_iname");
  const std::vector<std::string> & petsc_options_values = params.get<std::vector<std::string> >("petsc_options_value");

  // The options to store in FEProblem, start with any existing stored parameters
  MultiMooseEnum po("","", true);
  std::vector<std::string> pn;
  std::vector<std::string> pv;
  fe_problem.getPetscOptions(po, pn, pv);

  for (MooseEnumIterator it = petsc_options.begin(); it != petsc_options.end(); ++it)
  {
    /**
     * "-log_summary" cannot be used in the input file. This option needs to be set when PETSc is initialized
     * which happens before the parser is even created.  We'll throw an error if somebody attempts to add this option later.
     */
    if (*it == "-log_summary")
      mooseError("The PETSc option \"-log_summary\" can only be used on the command line.  Please remove it from the input file");

    // Warn about superseded PETSc options (Note: -snes is not a REAL option, but people used it in their input files)
    else
    {
      std::string help_string;
      if (*it == "-snes" || *it == "-snes_mf" || *it == "-snes_mf_operator")
        help_string = "Please set the solver type through \"solve_type\".";
      else if (*it == "-ksp_monitor")
        help_string = "Please use \"Outputs/console/type=Console Outputs/console/linear_residuals=true\"";

      if (help_string != "")
        mooseWarning("The PETSc option " << *it << " should not be used directly in a MOOSE input file. " << help_string);
    }

    if (find(po.begin(), po.end(), *it) == po.end())
      po.push_back(*it);
  }

  if (petsc_options_inames.size() != petsc_options_values.size())
    mooseError("PETSc names and options are not the same length");

  bool boomeramg_found = false;
  bool strong_threshold_found = false;
  std::string pc_description = "";
  for (unsigned int i = 0; i < petsc_options_inames.size(); i++)
  {
    if (find(pn.begin(), pn.end(), petsc_options_inames[i]) == pn.end())
    {
      pn.push_back(petsc_options_inames[i]);
      pv.push_back(petsc_options_values[i]);

      // Look for a pc description
      if (petsc_options_inames[i] == "-pc_type" || petsc_options_inames[i] == "-pc_sub_type" || petsc_options_inames[i] == "-pc_hypre_type")
        pc_description += petsc_options_values[i] + ' ';

      // This special case is common enough that we'd like to handle it for the user.
      if (petsc_options_inames[i] == "-pc_hypre_type" && petsc_options_values[i] == "boomeramg")
        boomeramg_found = true;
      if (petsc_options_inames[i] == "-pc_hypre_boomeramg_strong_threshold")
        strong_threshold_found = true;
    }
    else
    {
      for (unsigned int j = 0; j < pn.size(); j++)
        if (pn[j] == petsc_options_inames[i])
          pv[j] = petsc_options_values[i];
    }
  }

  // When running a 3D mesh with boomeramg, it is almost always best to supply a strong threshold value
  // We will provide that for the user here if they haven't supplied it themselves.
  if (boomeramg_found && !strong_threshold_found && fe_problem.mesh().dimension() == 3)
  {
    pn.push_back("-pc_hypre_boomeramg_strong_threshold");
    pv.push_back("0.7");
    pc_description += "strong_threshold: 0.7 (auto)";
  }

  // Set Preconditioner description
  fe_problem.setPreconditionerDescription(pc_description);

  // Store the parameters on FEProblem
  fe_problem.storePetscOptions(po, pn, pv);
}
#endif


// DEPRECATED CONSTRUCTOR
CreateExecutionerAction::CreateExecutionerAction(const std::string & deprecated_name, InputParameters params) :
    MooseObjectAction(deprecated_name, params)
{
}
