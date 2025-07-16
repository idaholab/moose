#ifdef MFEM_ENABLED

#include "ComplexEquationSystem.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

void
ComplexEquationSystem::Init(Moose::MFEM::ComplexGridFunctions & cpx_gridfunctions,
                            const Moose::MFEM::FESpaces & /*fespaces*/,
                            mfem::AssemblyLevel assembly_level)
{
  _assembly_level = assembly_level;

  for (auto & test_var_name : _test_var_names)
  {
    if (!cpx_gridfunctions.Has(test_var_name))
    {
      MFEM_ABORT("Test variable " << test_var_name
                                  << " requested by equation system during initialisation was "
                                     "not found in gridfunctions");
    }
    // Store pointers to variable FESpaces
    _test_pfespaces.push_back(cpx_gridfunctions.Get(test_var_name)->ParFESpace());
    // Create auxiliary gridfunctions for applying Dirichlet conditions
    _cxs.emplace_back(std::make_unique<mfem::ParComplexGridFunction>(
        cpx_gridfunctions.Get(test_var_name)->ParFESpace()));
    _cdxdts.emplace_back(std::make_unique<mfem::ParComplexGridFunction>(
        cpx_gridfunctions.Get(test_var_name)->ParFESpace()));
    _cpx_trial_variables.Register(test_var_name, cpx_gridfunctions.GetShared(test_var_name));
  }
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
    ApplyDomainLFIntegrators(test_var_name, clf, _cpx_kernels_map);
    ApplyBoundaryLFIntegrators(test_var_name, clf, _cpx_integrated_bc_map);
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
        test_var_name, test_var_name, slf, _cpx_integrated_bc_map);
    ApplyDomainBLFIntegrators<mfem::ParSesquilinearForm>(
        test_var_name, test_var_name, slf, _cpx_kernels_map);
    // Assemble
    slf->Assemble();
  }
}

void
ComplexEquationSystem::ApplyEssentialBCs()
{
  _ess_tdof_lists.resize(_test_var_names.size());
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    if (!_essential_bc_map.Has(test_var_name))
      continue;

    // Set default value of gridfunction used in essential BC. Values
    // overwritten in applyEssentialBCs
    mfem::ParComplexGridFunction & trial_gf(*(_cxs.at(i)));
    mfem::ParComplexGridFunction & trial_gf_time_derivatives(*(_cdxdts.at(i)));

    auto * const pmesh = _test_pfespaces.at(i)->GetParMesh();

    mooseAssert(pmesh, "parallel mesh is null");
    trial_gf = 0.0;
    trial_gf_time_derivatives = 0.0;

    auto bcs = _essential_bc_map.GetRef(test_var_name);
    mfem::Array<int> global_ess_markers(pmesh->bdr_attributes.Max());
    global_ess_markers = 0;

    for (auto & bc : bcs)
    {
      bc->ApplyComplexBC(trial_gf);

      mfem::Array<int> ess_bdrs(bc->getBoundaryMarkers());
      for (auto it = 0; it != pmesh->bdr_attributes.Max(); ++it)
      {
        global_ess_markers[it] = std::max(global_ess_markers[it], ess_bdrs[it]);
      }
    }
    trial_gf.FESpace()->GetEssentialTrueDofs(global_ess_markers, _ess_tdof_lists.at(i));
  }
}

void
ComplexEquationSystem::AddKernel(std::shared_ptr<MFEMComplexKernel> kernel)
{
  AddTestVariableNameIfMissing(kernel->getTestVariableName());
  AddTrialVariableNameIfMissing(kernel->getTrialVariableName());
  auto trial_var_name = kernel->getTrialVariableName();
  auto test_var_name = kernel->getTestVariableName();
  if (!_cpx_kernels_map.Has(test_var_name))
  {
    auto kernel_field_map =
        std::make_shared<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexKernel>>>>();
    _cpx_kernels_map.Register(test_var_name, std::move(kernel_field_map));
  }
  // Register new kernels map if not present for the test/trial variable
  // pair
  if (!_cpx_kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<MFEMComplexKernel>>>();
    _cpx_kernels_map.Get(test_var_name)->Register(trial_var_name, std::move(kernels));
  }
  _cpx_kernels_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(kernel));
}

void
ComplexEquationSystem::AddIntegratedBC(std::shared_ptr<MFEMComplexIntegratedBC> bc)
{
  AddTestVariableNameIfMissing(bc->getTestVariableName());
  AddTrialVariableNameIfMissing(bc->getTrialVariableName());
  auto trial_var_name = bc->getTrialVariableName();
  auto test_var_name = bc->getTestVariableName();
  if (!_cpx_integrated_bc_map.Has(test_var_name))
  {
    auto integrated_bc_field_map = std::make_shared<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>>();
    _cpx_integrated_bc_map.Register(test_var_name, std::move(integrated_bc_field_map));
  }
  // Register new integrated bc map if not present for the test/trial variable
  // pair
  if (!_cpx_integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMComplexIntegratedBC>>>();
    _cpx_integrated_bc_map.Get(test_var_name)->Register(trial_var_name, std::move(bcs));
  }
  _cpx_integrated_bc_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(bc));
}

void
ComplexEquationSystem::FormSystem(mfem::OperatorHandle & op,
                                  mfem::BlockVector & trueX,
                                  mfem::BlockVector & trueRHS)
{
  auto & test_var_name = _test_var_names.at(0);
  auto slf = _slfs.Get(test_var_name);
  auto clf = _clfs.Get(test_var_name);
  mfem::BlockVector aux_x, aux_rhs;
  mfem::OperatorPtr aux_a;

  slf->FormLinearSystem(_ess_tdof_lists.at(0), *(_cxs.at(0)), *clf, aux_a, aux_x, aux_rhs);

  trueX.GetBlock(0) = aux_x;
  trueRHS.GetBlock(0) = aux_rhs;
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  op.Reset(aux_a.Ptr());
  aux_a.SetOperatorOwner(false);
}

void
ComplexEquationSystem::FormLegacySystem(mfem::OperatorHandle & op,
                                        mfem::BlockVector & trueX,
                                        mfem::BlockVector & trueRHS)
{
  mooseError("ComplexEquationSystem does not support legacy assembly.");
  // Allocate block operator
  DeleteAllBlocks();
  _h_blocks.SetSize(_test_var_names.size(), _test_var_names.size());
  // Form diagonal blocks.
  for (const auto i : index_range(_test_var_names))
  {
    auto & test_var_name = _test_var_names.at(i);
    auto slf = _slfs.Get(test_var_name);
    auto clf = _clfs.Get(test_var_name);
    mfem::Vector aux_x, aux_rhs;
    mfem::OperatorPtr aux_a;
    slf->FormLinearSystem(_ess_tdof_lists.at(i), *(_cxs.at(i)), *clf, aux_a, aux_x, aux_rhs);
    _h_blocks(i, i) = aux_a.As<mfem::HypreParMatrix>();
    trueX.GetBlock(i) = aux_x;
    trueRHS.GetBlock(i) = aux_rhs;
  }

  // Sync memory
  for (const auto i : index_range(_test_var_names))
  {
    trueX.GetBlock(i).SyncAliasMemory(trueX);
    trueRHS.GetBlock(i).SyncAliasMemory(trueRHS);
  }

  // Create monolithic matrix
  op.Reset(mfem::HypreParMatrixFromBlocks(_h_blocks));
}

void
ComplexEquationSystem::RecoverFEMSolution(mfem::BlockVector & trueX,
                                          Moose::MFEM::ComplexGridFunctions & gridfunctions)
{
  for (const auto i : index_range(_trial_var_names))
  {
    auto & trial_var_name = _trial_var_names.at(i);
    trueX.GetBlock(i).SyncAliasMemory(trueX);
    gridfunctions.Get(trial_var_name)->Distribute(&(trueX.GetBlock(i)));
  }
}

}

#endif
