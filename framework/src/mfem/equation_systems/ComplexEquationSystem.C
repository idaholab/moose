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
    if ((_cmplx_kernels_map.Has(test_var_name) &&
         _cmplx_kernels_map.Get(test_var_name)->Has(test_var_name)) ||
        (_cmplx_integrated_bc_map.Has(test_var_name) &&
         _cmplx_integrated_bc_map.Get(test_var_name)->Has(test_var_name)))
    {
      // Apply kernels
      auto clf = _clfs.GetShared(test_var_name);
      ApplyDomainLFIntegrators(test_var_name, clf, _cmplx_kernels_map);
      ApplyBoundaryLFIntegrators(test_var_name, clf, _cmplx_integrated_bc_map);
      clf->Assemble();
    }
  }
}

void
ComplexEquationSystem::BuildBilinearForms()
{
  // Register bilinear forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    if ((_cmplx_kernels_map.Has(test_var_name) &&
         _cmplx_kernels_map.Get(test_var_name)->Has(test_var_name)) ||
        (_cmplx_integrated_bc_map.Has(test_var_name) &&
         _cmplx_integrated_bc_map.Get(test_var_name)->Has(test_var_name)))
    {
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
}

void
ComplexEquationSystem::BuildMixedBilinearForms()
{
  // Register mixed sesquilinear forms. Note that not all combinations may
  // have a kernel.

  // Create mslf for each test/coupled variable pair with an added kernel.
  // Mixed sesquilinear forms with coupled variables that are not trial variables are
  // associated with contributions from eliminated variables.
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    auto test_mslfs = std::make_shared<Moose::MFEM::NamedFieldsMap<ParMixedSesquilinearForm>>();
    for (const auto j : index_range(_coupled_var_names))
    {
      const auto & coupled_var_name = _coupled_var_names.at(j);
      auto mslf = std::make_shared<ParMixedSesquilinearForm>(_coupled_pfespaces.at(j),
                                                             _test_pfespaces.at(i));
      // Register MixedSesquilinearForm if kernels exist for it, and assemble
      // kernels
      if (_cmplx_kernels_map.Has(test_var_name) &&
          _cmplx_kernels_map.Get(test_var_name)->Has(coupled_var_name) &&
          test_var_name != coupled_var_name)
      {
        mslf->SetAssemblyLevel(_assembly_level);
        // Apply all mixed kernels with this test/trial pair
        ApplyDomainBLFIntegrators<ParMixedSesquilinearForm>(
            coupled_var_name, test_var_name, mslf, _cmplx_kernels_map);
        // Assemble mixed bilinear forms
        mslf->Assemble();
        // Register mixed bilinear forms associated with a single trial variable
        // for the current test variable
        test_mslfs->Register(coupled_var_name, mslf);
      }
    }
    // Register all mixed bilinear form sets associated with a single test
    // variable
    _mslfs.Register(test_var_name, test_mslfs);
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

  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    for (const auto j : index_range(_trial_var_names))
    {
      auto trial_var_name = _trial_var_names.at(j);

      mfem::Vector aux_x, aux_rhs;
      mfem::ParComplexLinearForm aux_lf(_test_pfespaces.at(i));
      mfem::OperatorHandle * aux_a = new mfem::OperatorHandle;
      aux_lf = 0.0;
      if (test_var_name == trial_var_name)
      {
        mooseAssert(i == j, "Trial and test variables must have the same ordering.");
        auto slf = _slfs.Get(test_var_name);
        auto clf = _clfs.Get(test_var_name);
        slf->FormLinearSystem(_ess_tdof_lists.at(j),
                              *(_cmplx_var_ess_constraints.at(j)),
                              *clf,
                              *aux_a,
                              aux_x,
                              aux_rhs);
        trueX.GetBlock(i) = aux_x;
      }
      else if (_mslfs.Has(test_var_name) && _mslfs.Get(test_var_name)->Has(trial_var_name))
      {
        auto mslf = _mslfs.Get(test_var_name)->Get(trial_var_name);
        mslf->FormRectangularLinearSystem(_ess_tdof_lists.at(j),
                                          _ess_tdof_lists.at(i),
                                          *(_cmplx_var_ess_constraints.at(j)),
                                          aux_lf,
                                          *aux_a,
                                          aux_x,
                                          aux_rhs);
      }
      else
        continue;

      trueRHS.GetBlock(i) += aux_rhs;
      _h_blocks(i, j) = aux_a->As<mfem::ComplexHypreParMatrix>()->GetSystemMatrix();
    }
  }

  // Sync memory
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  // Create monolithic matrix
  op.Reset(mfem::HypreParMatrixFromBlocks(_h_blocks));
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
