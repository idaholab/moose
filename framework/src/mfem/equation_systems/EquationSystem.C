#ifdef MFEM_ENABLED

#include "EquationSystem.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

EquationSystem::~EquationSystem() { _h_blocks.DeleteAll(); }

bool
EquationSystem::VectorContainsName(const std::vector<std::string> & the_vector,
                                   const std::string & name) const
{

  auto iter = std::find(the_vector.begin(), the_vector.end(), name);

  return (iter != the_vector.end());
}

void
EquationSystem::AddTrialVariableNameIfMissing(const std::string & trial_var_name)
{
  if (!VectorContainsName(_trial_var_names, trial_var_name))
  {
    _trial_var_names.push_back(trial_var_name);
  }
}

void
EquationSystem::AddTestVariableNameIfMissing(const std::string & test_var_name)
{
  if (!VectorContainsName(_test_var_names, test_var_name))
  {
    _test_var_names.push_back(test_var_name);
  }
}

void
EquationSystem::AddKernel(std::shared_ptr<MFEMKernel> kernel)
{
  AddTestVariableNameIfMissing(kernel->getTestVariableName());
  AddTrialVariableNameIfMissing(kernel->getTrialVariableName());
  auto trial_var_name = kernel->getTrialVariableName();
  auto test_var_name = kernel->getTestVariableName();
  if (!_kernels_map.Has(test_var_name))
  {
    auto kernel_field_map =
        std::make_shared<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>();
    _kernels_map.Register(test_var_name, std::move(kernel_field_map));
  }
  // Register new kernels map if not present for the test/trial variable
  // pair
  if (!_kernels_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<MFEMKernel>>>();
    _kernels_map.Get(test_var_name)->Register(trial_var_name, std::move(kernels));
  }
  _kernels_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(kernel));
}

void
EquationSystem::AddIntegratedBC(std::shared_ptr<MFEMIntegratedBC> bc)
{
  AddTestVariableNameIfMissing(bc->getTestVariableName());
  AddTrialVariableNameIfMissing(bc->getTrialVariableName());
  auto trial_var_name = bc->getTrialVariableName();
  auto test_var_name = bc->getTestVariableName();
  if (!_integrated_bc_map.Has(test_var_name))
  {
    auto integrated_bc_field_map = std::make_shared<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>>();
    _integrated_bc_map.Register(test_var_name, std::move(integrated_bc_field_map));
  }
  // Register new integrated bc map if not present for the test/trial variable
  // pair
  if (!_integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMIntegratedBC>>>();
    _integrated_bc_map.Get(test_var_name)->Register(trial_var_name, std::move(bcs));
  }
  _integrated_bc_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(bc));
}

void
EquationSystem::AddBC(const std::string & name, std::shared_ptr<MFEMBoundaryCondition> bc)
{
  if (_bc_map.Has(name))
  {
    const std::string error_message = "A boundary condition with the name " + name +
                                      " has already been added to the problem boundary conditions.";
    mfem::mfem_error(error_message.c_str());
  }
  _bc_map.Register(name, std::move(bc));
}

void
EquationSystem::ApplyBoundaryConditions()
{
  _ess_tdof_lists.resize(_test_var_names.size());
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    // Set default value of gridfunction used in essential BC. Values
    // overwritten in applyEssentialBCs
    *(_xs.at(i)) = 0.0;
    *(_dxdts.at(i)) = 0.0;
    auto * const par_mesh = _test_pfespaces.at(i)->GetParMesh();
    mooseAssert(par_mesh, "parallel mesh is null");
    _bc_map.ApplyEssentialBCs(test_var_name, _ess_tdof_lists.at(i), *(_xs.at(i)), *par_mesh);
  }
}

void
EquationSystem::FormLinearSystem(mfem::OperatorHandle & op,
                                 mfem::BlockVector & trueX,
                                 mfem::BlockVector & trueRHS)
{

  switch (_assembly_level)
  {
    case mfem::AssemblyLevel::LEGACY:
      FormLegacySystem(op, trueX, trueRHS);
      break;
    default:
      MFEM_VERIFY(_test_var_names.size() == 1,
                  "Non-legacy assembly is only supported for single-variable systems");
      MFEM_VERIFY(_test_var_names.size() == _trial_var_names.size(),
                  "Non-legacy assembly is only supported for square systems");
      FormSystem(op, trueX, trueRHS);
  }
}

void
EquationSystem::FormSystem(mfem::OperatorHandle & op,
                           mfem::BlockVector & trueX,
                           mfem::BlockVector & trueRHS)
{
  auto & test_var_name = _test_var_names.at(0);
  auto blf = _blfs.Get(test_var_name);
  auto lf = _lfs.Get(test_var_name);
  mfem::BlockVector aux_x, aux_rhs;
  mfem::OperatorPtr * aux_a = new mfem::OperatorPtr;

  blf->FormLinearSystem(_ess_tdof_lists.at(0), *(_xs.at(0)), *lf, *aux_a, aux_x, aux_rhs);

  trueX.GetBlock(0) = aux_x;
  trueRHS.GetBlock(0) = aux_rhs;
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  op.Reset(aux_a->Ptr());
}

void
EquationSystem::FormLegacySystem(mfem::OperatorHandle & op,
                                 mfem::BlockVector & trueX,
                                 mfem::BlockVector & trueRHS)
{

  // Allocate block operator
  _h_blocks.DeleteAll();
  _h_blocks.SetSize(_test_var_names.size(), _test_var_names.size());
  // Form diagonal blocks.
  for (const auto i : index_range(_test_var_names))
  {
    auto & test_var_name = _test_var_names.at(i);
    auto blf = _blfs.Get(test_var_name);
    auto lf = _lfs.Get(test_var_name);
    mfem::Vector aux_x, aux_rhs;
    mfem::HypreParMatrix * aux_a = new mfem::HypreParMatrix;
    // Ownership of aux_a goes to the blf
    blf->FormLinearSystem(_ess_tdof_lists.at(i), *(_xs.at(i)), *lf, *aux_a, aux_x, aux_rhs);
    _h_blocks(i, i) = aux_a;
    trueX.GetBlock(i) = aux_x;
    trueRHS.GetBlock(i) = aux_rhs;
  }

  // Form off-diagonal blocks
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    for (const auto j : index_range(_test_var_names))
    {
      auto trial_var_name = _test_var_names.at(j);

      mfem::Vector aux_x, aux_rhs;
      mfem::ParLinearForm aux_lf(_test_pfespaces.at(i));
      aux_lf = 0.0;
      if (_mblfs.Has(test_var_name) && _mblfs.Get(test_var_name)->Has(trial_var_name))
      {
        auto mblf = _mblfs.Get(test_var_name)->Get(trial_var_name);
        mfem::HypreParMatrix * aux_a = new mfem::HypreParMatrix;
        // Ownership of aux_a goes to the blf
        mblf->FormRectangularLinearSystem(_ess_tdof_lists.at(j),
                                          _ess_tdof_lists.at(i),
                                          *(_xs.at(j)),
                                          aux_lf,
                                          *aux_a,
                                          aux_x,
                                          aux_rhs);
        _h_blocks(i, j) = aux_a;
        trueRHS.GetBlock(i) += aux_rhs;
      }
    }
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
EquationSystem::BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS)
{
  height = trueX.Size();
  width = trueRHS.Size();
  FormLinearSystem(_jacobian, trueX, trueRHS);
}

void
EquationSystem::Mult(const mfem::Vector & x, mfem::Vector & residual) const
{
  _jacobian->Mult(x, residual);
  x.HostRead();
  residual.HostRead();
}

mfem::Operator &
EquationSystem::GetGradient(const mfem::Vector &) const
{
  return *_jacobian;
}

void
EquationSystem::RecoverFEMSolution(mfem::BlockVector & trueX,
                                   Moose::MFEM::GridFunctions & gridfunctions)
{
  for (const auto i : index_range(_trial_var_names))
  {
    auto & trial_var_name = _trial_var_names.at(i);
    trueX.GetBlock(i).SyncAliasMemory(trueX);
    gridfunctions.Get(trial_var_name)->Distribute(&(trueX.GetBlock(i)));
  }
}

void
EquationSystem::Init(Moose::MFEM::GridFunctions & gridfunctions,
                     const Moose::MFEM::FESpaces & /*fespaces*/,
                     mfem::AssemblyLevel assembly_level)
{
  _assembly_level = assembly_level;

  for (auto & test_var_name : _test_var_names)
  {
    if (!gridfunctions.Has(test_var_name))
    {
      MFEM_ABORT("Test variable " << test_var_name
                                  << " requested by equation system during initialisation was "
                                     "not found in gridfunctions");
    }
    // Store pointers to variable FESpaces
    _test_pfespaces.push_back(gridfunctions.Get(test_var_name)->ParFESpace());
    // Create auxiliary gridfunctions for applying Dirichlet conditions
    _xs.emplace_back(
        std::make_unique<mfem::ParGridFunction>(gridfunctions.Get(test_var_name)->ParFESpace()));
    _dxdts.emplace_back(
        std::make_unique<mfem::ParGridFunction>(gridfunctions.Get(test_var_name)->ParFESpace()));
    _trial_variables.Register(test_var_name, gridfunctions.GetShared(test_var_name));
  }
}

void
EquationSystem::BuildLinearForms()
{
  // Register linear forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    _lfs.Register(test_var_name, std::make_shared<mfem::ParLinearForm>(_test_pfespaces.at(i)));
    _lfs.GetRef(test_var_name) = 0.0;
  }
  // Apply boundary conditions
  ApplyBoundaryConditions();

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto lf = _lfs.GetShared(test_var_name);
    if (_kernels_map.Has(test_var_name))
    {
      ApplyDomainLFIntegrators(test_var_name, lf, _kernels_map);
    }
    if (_integrated_bc_map.Has(test_var_name))
    {
      ApplyBoundaryLFIntegrators(test_var_name, lf, _integrated_bc_map);
    }
    lf->Assemble();
  }
}

void
EquationSystem::BuildBilinearForms()
{
  // Register bilinear forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    _blfs.Register(test_var_name, std::make_shared<mfem::ParBilinearForm>(_test_pfespaces.at(i)));

    // Apply kernels
    auto blf = _blfs.GetShared(test_var_name);
    if (_integrated_bc_map.Has(test_var_name) &&
        _integrated_bc_map.Get(test_var_name)->Has(test_var_name))
    {
      ApplyBoundaryBLFIntegrators<mfem::ParBilinearForm>(
          test_var_name, test_var_name, blf, _integrated_bc_map);
    }
    if (_kernels_map.Has(test_var_name) && _kernels_map.Get(test_var_name)->Has(test_var_name))
    {
      blf->SetAssemblyLevel(_assembly_level);
      ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
          test_var_name, test_var_name, blf, _kernels_map);
    }
    // Assemble
    blf->Assemble();
  }
}

void
EquationSystem::BuildMixedBilinearForms()
{
  // Register mixed bilinear forms. Note that not all combinations may
  // have a kernel

  // Create mblf for each test/trial pair
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    auto test_mblfs = std::make_shared<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>();
    for (const auto j : index_range(_test_var_names))
    {
      auto trial_var_name = _test_var_names.at(j);

      // Register MixedBilinearForm if kernels exist for it, and assemble
      // kernels
      if (_kernels_map.Has(test_var_name) && _kernels_map.Get(test_var_name)->Has(trial_var_name) &&
          test_var_name != trial_var_name)
      {
        auto mblf = std::make_shared<mfem::ParMixedBilinearForm>(_test_pfespaces.at(j),
                                                                 _test_pfespaces.at(i));
        // Apply all mixed kernels with this test/trial pair
        ApplyDomainBLFIntegrators<mfem::ParMixedBilinearForm>(
            trial_var_name, test_var_name, mblf, _kernels_map);
        // Assemble mixed bilinear forms
        mblf->Assemble();
        // Register mixed bilinear forms associated with a single trial variable
        // for the current test variable
        test_mblfs->Register(trial_var_name, mblf);
      }
    }
    // Register all mixed bilinear form sets associated with a single test
    // variable
    _mblfs.Register(test_var_name, test_mblfs);
  }
}

void
EquationSystem::BuildEquationSystem()
{
  BuildBilinearForms();
  BuildMixedBilinearForms();
  BuildLinearForms();
}

TimeDependentEquationSystem::TimeDependentEquationSystem() : _dt_coef(1.0) {}

void
TimeDependentEquationSystem::AddTrialVariableNameIfMissing(const std::string & var_name)
{
  // The TimeDependentEquationSystem operator expects to act on a vector of variable time
  // derivatives
  std::string var_time_derivative_name = GetTimeDerivativeName(var_name);

  if (!VectorContainsName(_trial_var_names, var_time_derivative_name))
  {
    _trial_var_names.push_back(var_time_derivative_name);
    _trial_var_time_derivative_names.push_back(var_time_derivative_name);
  }
}

void
TimeDependentEquationSystem::SetTimeStep(double dt)
{
  if (fabs(dt - _dt_coef.constant) > 1.0e-12 * dt)
  {
    _dt_coef.constant = dt;
    for (auto test_var_name : _test_var_names)
    {
      auto blf = _blfs.Get(test_var_name);
      blf->Update();
      blf->Assemble();
    }
  }
}

void
TimeDependentEquationSystem::AddKernel(std::shared_ptr<MFEMKernel> kernel)
{
  if (kernel->getTrialVariableName() == GetTimeDerivativeName(kernel->getTestVariableName()))
  {
    auto trial_var_name = kernel->getTrialVariableName();
    auto test_var_name = kernel->getTestVariableName();
    AddTestVariableNameIfMissing(test_var_name);
    AddTrialVariableNameIfMissing(test_var_name);
    if (!_td_kernels_map.Has(test_var_name))
    {
      auto kernel_field_map =
          std::make_shared<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>();

      _td_kernels_map.Register(test_var_name, std::move(kernel_field_map));
    }
    // Register new kernels map if not present for the test/trial variable
    // pair
    if (!_td_kernels_map.Get(test_var_name)->Has(test_var_name))
    {
      auto kernels = std::make_shared<std::vector<std::shared_ptr<MFEMKernel>>>();

      _td_kernels_map.Get(test_var_name)->Register(test_var_name, std::move(kernels));
    }
    _td_kernels_map.GetRef(test_var_name).Get(test_var_name)->push_back(std::move(kernel));
  }
  else
  {
    EquationSystem::AddKernel(kernel);
  }
}

void
TimeDependentEquationSystem::BuildBilinearForms()
{
  EquationSystem::BuildBilinearForms();

  // Build and assemble bilinear forms acting on time derivatives
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);

    _td_blfs.Register(test_var_name,
                      std::make_shared<mfem::ParBilinearForm>(_test_pfespaces.at(i)));

    // Apply kernels to td_blf
    auto td_blf = _td_blfs.GetShared(test_var_name);
    if (_integrated_bc_map.Has(test_var_name) &&
        _integrated_bc_map.Get(test_var_name)->Has(test_var_name))
    {
      ApplyBoundaryBLFIntegrators<mfem::ParBilinearForm>(
          test_var_name, test_var_name, td_blf, _integrated_bc_map);
    }
    if (_td_kernels_map.Has(test_var_name) &&
        _td_kernels_map.Get(test_var_name)->Has(test_var_name))
    {
      td_blf->SetAssemblyLevel(_assembly_level);
      ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
          test_var_name, test_var_name, td_blf, _td_kernels_map);
    }

    // Recover and scale integrators from blf. This is to apply the dt*du/dt contributions from the
    // operator on the trial variable in the implicit integration scheme
    auto blf = _blfs.Get(test_var_name);
    auto integs = blf->GetDBFI();
    auto b_integs = blf->GetBBFI();
    auto markers = blf->GetBBFI_Marker();

    mfem::SumIntegrator * sum = new mfem::SumIntegrator;
    ScaleIntegrator * scaled_sum = new ScaleIntegrator(sum, _dt_coef.constant, false);

    for (int i = 0; i < integs->Size(); ++i)
    {
      sum->AddIntegrator(*integs[i]);
    }

    for (int i = 0; i < b_integs->Size(); ++i)
    {
      td_blf->AddBoundaryIntegrator(new ScaleIntegrator(*b_integs[i], _dt_coef.constant, false),
                                    *(*markers[i]));
    }

    // scaled_sum is owned by td_blf
    td_blf->AddDomainIntegrator(scaled_sum);

    // Assemble form
    td_blf->Assemble();
  }
}

void
TimeDependentEquationSystem::FormLegacySystem(mfem::OperatorHandle & op,
                                              mfem::BlockVector & truedXdt,
                                              mfem::BlockVector & trueRHS)
{

  // Allocate block operator
  _h_blocks.DeleteAll();
  _h_blocks.SetSize(_test_var_names.size(), _test_var_names.size());
  // Form diagonal blocks.
  for (const auto i : index_range(_test_var_names))
  {
    auto & test_var_name = _test_var_names.at(i);
    auto td_blf = _td_blfs.Get(test_var_name);
    auto blf = _blfs.Get(test_var_name);
    auto lf = _lfs.Get(test_var_name);
    // if implicit, add contribution to linear form from terms involving state
    // variable at previous timestep: {
    blf->AddMult(*_trial_variables.Get(test_var_name), *lf, -1.0);
    // }
    mfem::Vector aux_x, aux_rhs;
    // Update solution values on Dirichlet values to be in terms of du/dt instead of u
    mfem::Vector bc_x = *(_xs.at(i).get());
    bc_x -= *_trial_variables.Get(test_var_name);
    bc_x /= _dt_coef.constant;

    // Form linear system for operator acting on vector of du/dt
    mfem::HypreParMatrix * aux_a = new mfem::HypreParMatrix;
    // Ownership of aux_a goes to the blf
    td_blf->FormLinearSystem(_ess_tdof_lists.at(i), bc_x, *lf, *aux_a, aux_x, aux_rhs);
    _h_blocks(i, i) = aux_a;
    truedXdt.GetBlock(i) = aux_x;
    trueRHS.GetBlock(i) = aux_rhs;
  }

  truedXdt.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  // Create monolithic matrix
  op.Reset(mfem::HypreParMatrixFromBlocks(_h_blocks));
}

void
TimeDependentEquationSystem::FormSystem(mfem::OperatorHandle & op,
                                        mfem::BlockVector & truedXdt,
                                        mfem::BlockVector & trueRHS)
{
  auto & test_var_name = _test_var_names.at(0);
  auto td_blf = _td_blfs.Get(test_var_name);
  auto blf = _blfs.Get(test_var_name);
  auto lf = _lfs.Get(test_var_name);
  // if implicit, add contribution to linear form from terms involving state
  // variable at previous timestep: {

  // The AddMult method in mfem::BilinearForm is not defined for non-legacy assembly
  mfem::Vector lf_prev(lf->Size());
  blf->Mult(*_trial_variables.Get(test_var_name), lf_prev);
  *lf -= lf_prev;
  // }
  mfem::Vector aux_x, aux_rhs;
  // Update solution values on Dirichlet values to be in terms of du/dt instead of u
  mfem::Vector bc_x = *(_xs.at(0).get());
  bc_x -= *_trial_variables.Get(test_var_name);
  bc_x /= _dt_coef.constant;

  // Form linear system for operator acting on vector of du/dt
  mfem::OperatorPtr * aux_a = new mfem::OperatorPtr;
  // Ownership of aux_a goes to the blf
  td_blf->FormLinearSystem(_ess_tdof_lists.at(0), bc_x, *lf, *aux_a, aux_x, aux_rhs);

  truedXdt.GetBlock(0) = aux_x;
  trueRHS.GetBlock(0) = aux_rhs;
  truedXdt.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  // Create monolithic matrix
  op.Reset(aux_a->Ptr());
}

void
TimeDependentEquationSystem::UpdateEquationSystem()
{
  BuildBilinearForms();
  BuildMixedBilinearForms();
  BuildLinearForms();
}

} // namespace Moose::MFEM

#endif
