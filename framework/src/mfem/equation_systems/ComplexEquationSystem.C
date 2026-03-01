#ifdef MOOSE_MFEM_ENABLED

#include "ComplexEquationSystem.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

void
ComplexEquationSystem::Init(GridFunctions & gridfunctions,
                            ComplexGridFunctions & cmplx_gridfunctions,
                            mfem::AssemblyLevel assembly_level)
{
  _assembly_level = assembly_level;

  if (gridfunctions.size())
    mooseError("Mixing real and complex variables is currently not supported.");

  for (auto & test_var_name : _test_var_names)
  {
    if (!cmplx_gridfunctions.Has(test_var_name))
    {
      mooseError("MFEM complex variable ",
                 test_var_name,
                 " requested by equation system during initialization was "
                 "not found in gridfunctions");
    }
    // Store pointers to test FESpaces
    _test_pfespaces.push_back(cmplx_gridfunctions.Get(test_var_name)->ParFESpace());
    // Create auxiliary gridfunctions for storing essential constraints from Dirichlet conditions
    _cmplx_var_ess_constraints.emplace_back(std::make_unique<mfem::ParComplexGridFunction>(
        cmplx_gridfunctions.Get(test_var_name)->ParFESpace()));
  }

  // Store pointers to FESpaces of all coupled variables
  for (auto & coupled_var_name : _coupled_var_names)
    _coupled_pfespaces.push_back(cmplx_gridfunctions.Get(coupled_var_name)->ParFESpace());

  // Extract which coupled variables are to be trivially eliminated and which are trial variables
  SetTrialVariableNames();

  // Store pointers to coupled variable ComplexGridFunctions that are to be eliminated prior to
  // forming the jacobian
  for (auto & eliminated_var_name : _eliminated_var_names)
    _cmplx_eliminated_variables.Register(eliminated_var_name,
                                         cmplx_gridfunctions.GetShared(eliminated_var_name));
}

void
ComplexEquationSystem::BuildEquationSystem()
{
  BuildBilinearForms();
  BuildLinearForms();
}

void
ComplexEquationSystem::BuildLinearForms()
{
  // Register linear forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    _clfs.Register(test_var_name,
                   std::make_shared<mfem::ParComplexLinearForm>(_test_pfespaces.at(i)));
    _clfs.GetRef(test_var_name) = 0.0;
  }
  // Apply boundary conditions
  ApplyEssentialBCs();

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto clf = _clfs.GetShared(test_var_name);
    ApplyDomainLFIntegrators(test_var_name, clf, _cmplx_kernels_map);
    ApplyBoundaryLFIntegrators(test_var_name, clf, _cmplx_integrated_bc_map);
    clf->Assemble();
  }
}

void
ComplexEquationSystem::BuildBilinearForms()
{
  // Register bilinear forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    _slfs.Register(test_var_name,
                   std::make_shared<mfem::ParSesquilinearForm>(_test_pfespaces.at(i)));

    // Apply kernels
    auto slf = _slfs.GetShared(test_var_name);
    slf->SetAssemblyLevel(_assembly_level);
    ApplyBoundaryBLFIntegrators<mfem::ParSesquilinearForm>(
        test_var_name, test_var_name, slf, _cmplx_integrated_bc_map);
    ApplyDomainBLFIntegrators<mfem::ParSesquilinearForm>(
        test_var_name, test_var_name, slf, _cmplx_kernels_map);
    // Assemble
    slf->Assemble();
  }
}

void
ComplexEquationSystem::ApplyComplexEssentialBC(const std::string & var_name,
                                               mfem::ParComplexGridFunction & trial_gf,
                                               mfem::Array<int> & global_ess_markers)
{
  if (_cmplx_essential_bc_map.Has(var_name))
  {
    auto & bcs = _cmplx_essential_bc_map.GetRef(var_name);
    for (auto & bc : bcs)
    {
      // Set constrained DoFs values on essential boundaries
      bc->ApplyBC(trial_gf);
      // Fetch marker array labelling essential boundaries of current BC
      mfem::Array<int> ess_bdrs(bc->getBoundaryMarkers());
      // Add these boundary markers to the set of markers labelling all essential boundaries
      for (const auto i : make_range(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max()))
        global_ess_markers[i] = std::max(global_ess_markers[i], ess_bdrs[i]);
    }
  }
}

void
ComplexEquationSystem::ApplyEssentialBCs()
{
  _ess_tdof_lists.resize(_trial_var_names.size());
  for (const auto i : index_range(_trial_var_names))
  {
    const auto & trial_var_name = _trial_var_names.at(i);
    mfem::ParComplexGridFunction & trial_gf = *_cmplx_var_ess_constraints.at(i);
    trial_gf = std::complex<mfem::real_t>(0, 0);
    mfem::Array<int> global_ess_markers(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
    global_ess_markers = 0;
    // Set strongly constrained DoFs of trial_gf on essential boundaries and add markers for all
    // essential boundaries to the global_ess_markers array
    ApplyComplexEssentialBC(trial_var_name, trial_gf, global_ess_markers);
    trial_gf.FESpace()->GetEssentialTrueDofs(global_ess_markers, _ess_tdof_lists.at(i));
  }
}

void
ComplexEquationSystem::AddComplexKernel(std::shared_ptr<MFEMComplexKernel> kernel)
{
  const auto & trial_var_name = kernel->getTrialVariableName();
  const auto & test_var_name = kernel->getTestVariableName();
  AddCoupledVariableNameIfMissing(trial_var_name);
  AddTestVariableNameIfMissing(test_var_name);
  // Register new complex kernels map if not present for the test variable
  if (!_cmplx_kernels_map.Has(test_var_name))
  {
    auto kernel_field_map =
        std::make_shared<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>>();
    _cmplx_kernels_map.Register(test_var_name, std::move(kernel_field_map));
  }
  // Register new complex kernels map if not present for the test/trial variable pair
  if (!_cmplx_kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<MFEMComplexKernel>>>();
    _cmplx_kernels_map.Get(test_var_name)->Register(trial_var_name, std::move(kernels));
  }
  _cmplx_kernels_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(kernel));
}

void
ComplexEquationSystem::AddComplexIntegratedBC(std::shared_ptr<MFEMComplexIntegratedBC> bc)
{
  const auto & trial_var_name = bc->getTrialVariableName();
  const auto & test_var_name = bc->getTestVariableName();
  AddCoupledVariableNameIfMissing(trial_var_name);
  AddTestVariableNameIfMissing(test_var_name);
  // Register new complex integrated bc map if not present for the test variable
  if (!_cmplx_integrated_bc_map.Has(test_var_name))
  {
    auto integrated_bc_field_map =
        std::make_shared<NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>>();
    _cmplx_integrated_bc_map.Register(test_var_name, std::move(integrated_bc_field_map));
  }
  // Register new complex integrated bc map if not present for the test/trial variable pair
  if (!_cmplx_integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>();
    _cmplx_integrated_bc_map.Get(test_var_name)->Register(trial_var_name, std::move(bcs));
  }
  _cmplx_integrated_bc_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(bc));
}

void
ComplexEquationSystem::AddComplexEssentialBCs(std::shared_ptr<MFEMComplexEssentialBC> bc)
{
  const auto & test_var_name = bc->getTestVariableName();
  AddTestVariableNameIfMissing(test_var_name);
  // Register new complex essential bc map if not present for the test variable
  if (!_cmplx_essential_bc_map.Has(test_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMComplexEssentialBC>>>();
    _cmplx_essential_bc_map.Register(test_var_name, std::move(bcs));
  }
  _cmplx_essential_bc_map.GetRef(test_var_name).push_back(std::move(bc));
}

void
ComplexEquationSystem::FormSystemOperator(mfem::OperatorHandle & op,
                                          mfem::BlockVector & trueX,
                                          mfem::BlockVector & trueRHS)
{
  auto & test_var_name = _test_var_names.at(0);
  mfem::Vector aux_x, aux_rhs;
  mfem::OperatorPtr aux_a;

  auto slf = _slfs.Get(test_var_name);
  slf->FormLinearSystem(_ess_tdof_lists.at(0),
                        *_cmplx_var_ess_constraints.at(0),
                        *_clfs.Get(test_var_name),
                        aux_a,
                        aux_x,
                        aux_rhs,
                        /*copy_interior=*/true);

  trueX.GetBlock(0) = aux_x;
  trueRHS.GetBlock(0) = aux_rhs;
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  op.Reset(aux_a.Ptr());
  aux_a.SetOperatorOwner(false);
}

void
ComplexEquationSystem::FormSystemMatrix(mfem::OperatorHandle & op,
                                        mfem::BlockVector & trueX,
                                        mfem::BlockVector & trueRHS)
{

  // Allocate block operator
  DeleteAllBlocks();
  _h_blocks.SetSize(_test_var_names.size(), _trial_var_names.size());
  _h_blocks = nullptr;
  // Zero out RHS and sync memory
  trueRHS = 0.0;
  trueRHS.SyncToBlocks();

  // Form diagonal blocks.
  for (const auto i : index_range(_test_var_names))
  {
    auto & test_var_name = _test_var_names.at(i);

    mfem::Vector aux_x, aux_rhs;
    mfem::OperatorHandle aux_a;

    auto slf = _slfs.Get(test_var_name);
    slf->FormLinearSystem(_ess_tdof_lists.at(i),
                          *_cmplx_var_ess_constraints.at(i),
                          *_clfs.Get(test_var_name),
                          aux_a,
                          aux_x,
                          aux_rhs,
                          /*copy_interior=*/true);
    trueX.GetBlock(i) = aux_x;
    trueRHS.GetBlock(i) = aux_rhs;
    _h_blocks(i, i) = aux_a.As<mfem::ComplexHypreParMatrix>()->GetSystemMatrix();
  }
  // Sync memory
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  // Create monolithic matrix
  op.Reset(mfem::HypreParMatrixFromBlocks(_h_blocks));
}

// Equation system Mult
void
ComplexEquationSystem::Mult(const mfem::Vector & x, mfem::Vector & residual) const
{
  _jacobian->Mult(x, residual);
  x.HostRead();
  residual.HostRead();
}

void
ComplexEquationSystem::RecoverComplexFEMSolution(
    mfem::BlockVector & trueX,
    Moose::MFEM::GridFunctions & /*gridfunctions*/,
    Moose::MFEM::ComplexGridFunctions & cmplx_gridfunctions)
{
  for (const auto i : index_range(_trial_var_names))
    cmplx_gridfunctions.Get(_trial_var_names.at(i))->Distribute(&(trueX.GetBlock(i)));
}

}

#endif
