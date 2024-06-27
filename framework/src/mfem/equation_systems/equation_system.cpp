#include "equation_system.hpp"

namespace hephaestus
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
EquationSystem::AddKernel(const std::string & test_var_name,
                          std::shared_ptr<ParBilinearFormKernel> blf_kernel)
{
  AddTestVariableNameIfMissing(test_var_name);

  if (!_blf_kernels_map.Has(test_var_name))
  {
    // 1. Create kernels vector.
    auto kernels = std::make_shared<std::vector<std::shared_ptr<ParBilinearFormKernel>>>();

    // 2. Register with map to prevent leaks.
    _blf_kernels_map.Register(test_var_name, std::move(kernels));
  }

  _blf_kernels_map.GetRef(test_var_name).push_back(std::move(blf_kernel));
}

void
EquationSystem::AddKernel(const std::string & test_var_name,
                          std::shared_ptr<ParLinearFormKernel> lf_kernel)
{
  AddTestVariableNameIfMissing(test_var_name);

  if (!_lf_kernels_map.Has(test_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<ParLinearFormKernel>>>();

    _lf_kernels_map.Register(test_var_name, std::move(kernels));
  }

  _lf_kernels_map.GetRef(test_var_name).push_back(std::move(lf_kernel));
}

void
EquationSystem::AddKernel(const std::string & test_var_name,
                          std::shared_ptr<ParNonlinearFormKernel> nlf_kernel)
{
  AddTestVariableNameIfMissing(test_var_name);

  if (!_nlf_kernels_map.Has(test_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<ParNonlinearFormKernel>>>();

    _nlf_kernels_map.Register(test_var_name, std::move(kernels));
  }

  _nlf_kernels_map.GetRef(test_var_name).push_back(std::move(nlf_kernel));
}

void
EquationSystem::AddKernel(const std::string & trial_var_name,
                          const std::string & test_var_name,
                          std::shared_ptr<ParMixedBilinearFormKernel> mblf_kernel)
{
  AddTestVariableNameIfMissing(test_var_name);

  // Register new mblf kernels map if not present for this test variable
  if (!_mblf_kernels_map_map.Has(test_var_name))
  {
    auto kernel_field_map = std::make_shared<
        hephaestus::NamedFieldsMap<std::vector<std::shared_ptr<ParMixedBilinearFormKernel>>>>();

    _mblf_kernels_map_map.Register(test_var_name, std::move(kernel_field_map));
  }

  // Register new mblf kernels map if not present for the test/trial variable
  // pair
  if (!_mblf_kernels_map_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto kernels = std::make_shared<std::vector<std::shared_ptr<ParMixedBilinearFormKernel>>>();

    _mblf_kernels_map_map.Get(test_var_name)->Register(trial_var_name, std::move(kernels));
  }

  _mblf_kernels_map_map.GetRef(test_var_name)
      .Get(trial_var_name)
      ->push_back(std::move(mblf_kernel));
}

void
EquationSystem::ApplyBoundaryConditions(hephaestus::BCMap & bc_map)
{
  _ess_tdof_lists.resize(_test_var_names.size());
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    // Set default value of gridfunction used in essential BC. Values
    // overwritten in applyEssentialBCs
    *(_xs.at(i)) = 0.0;
    bc_map.ApplyEssentialBCs(
        test_var_name, _ess_tdof_lists.at(i), *(_xs.at(i)), _test_pfespaces.at(i)->GetParMesh());
    bc_map.ApplyIntegratedBCs(
        test_var_name, _lfs.GetRef(test_var_name), _test_pfespaces.at(i)->GetParMesh());
  }
}
void
EquationSystem::FormLinearSystem(mfem::OperatorHandle & op,
                                 mfem::BlockVector & trueX,
                                 mfem::BlockVector & trueRHS)
{

  // Allocate block operator
  _h_blocks.DeleteAll();
  _h_blocks.SetSize(_test_var_names.size(), _test_var_names.size());
  // Form diagonal blocks.
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto & test_var_name = _test_var_names.at(i);
    auto blf = _blfs.Get(test_var_name);
    auto lf = _lfs.Get(test_var_name);
    mfem::Vector aux_x, aux_rhs;
    _h_blocks(i, i) = new mfem::HypreParMatrix;
    blf->FormLinearSystem(
        _ess_tdof_lists.at(i), *(_xs.at(i)), *lf, *_h_blocks(i, i), aux_x, aux_rhs);
    trueX.GetBlock(i) = aux_x;
    trueRHS.GetBlock(i) = aux_rhs;
  }

  // Form off-diagonal blocks
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    for (int j = 0; j < _test_var_names.size(); j++)
    {
      auto trial_var_name = _test_var_names.at(j);

      mfem::Vector aux_x, aux_rhs;
      mfem::ParLinearForm aux_lf(_test_pfespaces.at(i));
      aux_lf = 0.0;
      if (_mblfs.Has(test_var_name) && _mblfs.Get(test_var_name)->Has(trial_var_name))
      {
        auto mblf = _mblfs.Get(test_var_name)->Get(trial_var_name);
        _h_blocks(i, j) = new mfem::HypreParMatrix;
        mblf->FormRectangularLinearSystem(_ess_tdof_lists.at(j),
                                          _ess_tdof_lists.at(i),
                                          *(_xs.at(j)),
                                          aux_lf,
                                          *_h_blocks(i, j),
                                          aux_x,
                                          aux_rhs);
        trueRHS.GetBlock(i) += aux_rhs;
      }
    }
  }
  // Sync memory
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    trueX.GetBlock(0).SyncAliasMemory(trueX);
    trueRHS.GetBlock(0).SyncAliasMemory(trueRHS);
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
}

mfem::Operator &
EquationSystem::GetGradient(const mfem::Vector & u) const
{
  return *_jacobian;
}

void
EquationSystem::RecoverFEMSolution(mfem::BlockVector & trueX,
                                   hephaestus::GridFunctions & gridfunctions)
{
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto & test_var_name = _test_var_names.at(i);
    trueX.GetBlock(i).SyncAliasMemory(trueX);
    gridfunctions.Get(test_var_name)->Distribute(&(trueX.GetBlock(i)));
  }
}

void
EquationSystem::Init(hephaestus::GridFunctions & gridfunctions,
                     const hephaestus::FESpaces & fespaces,
                     hephaestus::BCMap & bc_map,
                     hephaestus::Coefficients & coefficients)
{

  // Add optional kernels to the EquationSystem
  AddKernels();

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
  }

  // Initialise bilinear forms

  for (const auto & [test_var_name, blf_kernels] : _blf_kernels_map)
  {
    for (auto & i : *blf_kernels)
    {
      i->Init(gridfunctions, fespaces, bc_map, coefficients);
    }
  }
  // Initialise linear form kernels
  for (const auto & [test_var_name, lf_kernels] : _lf_kernels_map)
  {
    for (auto & i : *lf_kernels)
    {
      i->Init(gridfunctions, fespaces, bc_map, coefficients);
    }
  }
  // Initialise nonlinear form kernels
  for (const auto & [test_var_name, nlf_kernels] : _nlf_kernels_map)
  {
    for (auto & i : *nlf_kernels)
    {
      i->Init(gridfunctions, fespaces, bc_map, coefficients);
    }
  }
  // Initialise mixed bilinear form kernels
  for (const auto & [test_var_name, mblf_kernels_map] : _mblf_kernels_map_map)
  {
    for (const auto & [trial_var_name, mblf_kernels] : *mblf_kernels_map)
    {
      for (auto & i : *mblf_kernels)
      {
        i->Init(gridfunctions, fespaces, bc_map, coefficients);
      }
    }
  }
}

void
EquationSystem::BuildLinearForms(hephaestus::BCMap & bc_map, hephaestus::Sources & sources)
{
  // Register linear forms
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    _lfs.Register(test_var_name, std::make_shared<mfem::ParLinearForm>(_test_pfespaces.at(i)));
    _lfs.GetRef(test_var_name) = 0.0;
  }
  // Apply boundary conditions
  ApplyBoundaryConditions(bc_map);

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto lf = _lfs.Get(test_var_name);
    // Assemble. Must be done before applying kernels that add to lf.
    lf->Assemble();

    if (_lf_kernels_map.Has(test_var_name))
    {
      auto lf_kernels = _lf_kernels_map.GetRef(test_var_name);

      for (auto & lf_kernel : lf_kernels)
      {
        lf_kernel->Apply(lf);
      }
    }

    if (test_var_name == _test_var_names.at(0))
    {
      sources.Apply(lf);
    }
  }
}

void
EquationSystem::BuildBilinearForms()
{
  // Register bilinear forms
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    _blfs.Register(test_var_name, std::make_shared<mfem::ParBilinearForm>(_test_pfespaces.at(i)));

    // Apply kernels
    auto blf = _blfs.Get(test_var_name);
    if (_blf_kernels_map.Has(test_var_name))
    {
      auto blf_kernels = _blf_kernels_map.GetRef(test_var_name);

      for (auto & blf_kernel : blf_kernels)
      {
        blf_kernel->Apply(blf);
      }
    }
    // Assemble
    blf->Assemble();
  }
}

void
EquationSystem::BuildMixedBilinearForms()
{
  // Register mixed linear forms. Note that not all combinations may
  // have a kernel

  // Create mblf for each test/trial pair
  for (int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    auto test_mblfs = std::make_shared<hephaestus::NamedFieldsMap<mfem::ParMixedBilinearForm>>();
    for (int j = 0; j < _test_var_names.size(); j++)
    {
      auto trial_var_name = _test_var_names.at(j);

      // Register MixedBilinearForm if kernels exist for it, and assemble
      // kernels
      if (_mblf_kernels_map_map.Has(test_var_name) &&
          _mblf_kernels_map_map.Get(test_var_name)->Has(trial_var_name))
      {
        auto mblf_kernels = _mblf_kernels_map_map.GetRef(test_var_name).GetRef(trial_var_name);
        auto mblf = std::make_shared<mfem::ParMixedBilinearForm>(_test_pfespaces.at(j),
                                                                 _test_pfespaces.at(i));
        // Apply all mixed kernels with this test/trial pair
        for (auto & mblf_kernel : mblf_kernels)
        {
          mblf_kernel->Apply(mblf.get());
        }
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
EquationSystem::BuildEquationSystem(hephaestus::BCMap & bc_map, hephaestus::Sources & sources)
{
  BuildLinearForms(bc_map, sources);
  BuildBilinearForms();
  BuildMixedBilinearForms();
}

TimeDependentEquationSystem::TimeDependentEquationSystem() : _dt_coef(1.0) {}

void
TimeDependentEquationSystem::AddTrialVariableNameIfMissing(const std::string & var_name)
{
  EquationSystem::AddTrialVariableNameIfMissing(var_name);
  std::string var_time_derivative_name = GetTimeDerivativeName(var_name);
  if (std::find(_trial_var_time_derivative_names.begin(),
                _trial_var_time_derivative_names.end(),
                var_time_derivative_name) == _trial_var_time_derivative_names.end())
  {
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
TimeDependentEquationSystem::UpdateEquationSystem(hephaestus::BCMap & bc_map,
                                                  hephaestus::Sources & sources)
{
  BuildLinearForms(bc_map, sources);
  BuildBilinearForms();
  BuildMixedBilinearForms();
}

} // namespace hephaestus
