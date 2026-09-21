//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreBoomerAMG.h"
#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::HypreBoomerAMG>::validParams();
  params.addClassDescription("Hypre BoomerAMG solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMFESpaceName>(
      "fespace",
      "Vector H1 FESpace of the unknown, required when vector_treatment is 'by_component' or "
      "'rigid_body_modes'.");
  MooseEnum vector_treatment("auto scalar by_component rigid_body_modes", "auto");
  params.addParam<MooseEnum>(
      "vector_treatment",
      vector_treatment,
      "How BoomerAMG treats the components of a vector unknown. 'scalar' coarsens all degrees of "
      "freedom together as one unknown. 'by_component' coarsens each component separately and "
      "does not interpolate between them (hypre's 'unknown' approach, SetSystemsOptions). "
      "'rigid_body_modes' also adds the rigid body modes of the fespace to the interpolation "
      "(hypre's GM/LN approach, SetElasticityOptions), and is only applied on the CPU. 'auto' is "
      "'rigid_body_modes' if fespace is set and 'scalar' otherwise.");
  params.addParam<mfem::real_t>(
      "strength_threshold",
      0.25,
      "HypreBoomerAMG strong threshold. Defaults to 0.25, or to 0.5 when vector_treatment is "
      "'by_component' or 'rigid_body_modes'.");
  params.addParam<int>(
      "aggressive_coarsening_levels",
      0,
      "Number of levels, counted from the finest, on which to apply aggressive coarsening. "
      "Aggressive coarsening produces a cheaper hierarchy at the cost of more iterations.");
  MooseEnum errmode("ignore=0 warn=1 abort=2", "abort");
  params.addParam<MooseEnum>("error_mode", errmode, "Set the behavior for treating hypre errors.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::HypreBoomerAMG>(parameters),
    _mfem_fespace(
        isParamSetByUser("fespace")
            ? getMFEMProblem()
                  .getMFEMObject<MFEMFESpace>("MFEMFESpace", getParam<MFEMFESpaceName>("fespace"))
                  .getFESpace()
            : nullptr)
{
  _vector_treatment = getParam<MooseEnum>("vector_treatment").getEnum<VectorTreatment>();
  if (_vector_treatment == VectorTreatment::AUTO)
    _vector_treatment = _mfem_fespace ? VectorTreatment::RIGID_BODY_MODES : VectorTreatment::SCALAR;

  if (_vector_treatment == VectorTreatment::SCALAR && _mfem_fespace)
    paramError("fespace",
               "is only used when vector_treatment is 'by_component' or 'rigid_body_modes'.");
  if (_vector_treatment != VectorTreatment::SCALAR)
  {
    if (!_mfem_fespace)
      paramError("vector_treatment",
                 "'by_component' and 'rigid_body_modes' require fespace to be set.");
    if (_mfem_fespace->GetVDim() < 2)
      paramError("fespace",
                 "must be a vector space when vector_treatment is 'by_component' or "
                 "'rigid_body_modes'.");
    // mfem::HypreBoomerAMG builds the component map of a byNODES space from the size of its
    // operator, which a preconditioner does not have when these options are applied.
    if (_mfem_fespace->GetOrdering() != mfem::Ordering::byVDIM)
      paramError("fespace",
                 "must have ordering = VDIM when vector_treatment is 'by_component' or "
                 "'rigid_body_modes'.");
  }

  ConstructSolver();
}

MFEMHypreBoomerAMG::~MFEMHypreBoomerAMG() { _solver.reset(); }

void
MFEMHypreBoomerAMG::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreBoomerAMG>();
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreBoomerAMG::SetSolverParameters(mfem::HypreBoomerAMG & solver)
{
  solver.iterative_mode = getParam<bool>("use_initial_guess");
  solver.SetTol(getParam<mfem::real_t>("l_tol"));
  solver.SetMaxIter(getParam<int>("l_max_its"));
  solver.SetPrintLevel(getParam<int>("print_level"));
  solver.SetErrorMode(mfem::HypreSolver::ErrorMode(int(getParam<MooseEnum>("error_mode"))));

  switch (_vector_treatment)
  {
    case VectorTreatment::BY_COMPONENT:
      solver.SetSystemsOptions(_mfem_fespace->GetVDim(), /*order_bynodes=*/false);
      break;
    case VectorTreatment::RIGID_BODY_MODES:
      if (!mfem::HypreUsingGPU())
        solver.SetElasticityOptions(_mfem_fespace.get());
      break;
    default:
      break;
  }

  // Both option sets above reset the strength threshold to 0.5, so it is applied after them,
  // and only when given, to leave each set's own default in place otherwise.
  if (isParamSetByUser("strength_threshold"))
    solver.SetStrengthThresh(getParam<mfem::real_t>("strength_threshold"));

  if (isParamSetByUser("aggressive_coarsening_levels"))
    solver.SetAggressiveCoarsening(getParam<int>("aggressive_coarsening_levels"));
}

#endif
