//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html */

#ifdef MOOSE_MFEM_ENABLED

#include "EquationSystem.h"
#include "libmesh/int_range.h"

namespace Moose::MFEM
{

EquationSystem::~EquationSystem()
{
  if(_gfuncs != NULL) delete _gfuncs;
  if(_block_true_offsets != NULL) delete _block_true_offsets;
  DeleteAllBlocks();
}

void
EquationSystem::DeleteAllBlocks()
{
  for (const auto i : make_range(_h_blocks.NumRows()))
    for (const auto j : make_range(_h_blocks.NumCols()))
      delete _h_blocks(i, j);
  _h_blocks.DeleteAll();
}

bool
EquationSystem::VectorContainsName(const std::vector<std::string> & the_vector,
                                   const std::string & name) const
{
  return std::find(the_vector.begin(), the_vector.end(), name) != the_vector.end();
}

void
EquationSystem::AddCoupledVariableNameIfMissing(const std::string & coupled_var_name)
{
  if (!VectorContainsName(_coupled_var_names, coupled_var_name))
    _coupled_var_names.push_back(coupled_var_name);
}

void
EquationSystem::AddEliminatedVariableNameIfMissing(const std::string & eliminated_var_name)
{
  if (!VectorContainsName(_eliminated_var_names, eliminated_var_name))
    _eliminated_var_names.push_back(eliminated_var_name);
}

void
EquationSystem::AddTestVariableNameIfMissing(const std::string & test_var_name)
{
  if (!VectorContainsName(_test_var_names, test_var_name))
    _test_var_names.push_back(test_var_name);
}

void
EquationSystem::SetTrialVariableNames()
{
  // If a coupled variable has an equation associated with it,
  // add it to the set of trial variables.
  for (const auto & test_var_name : _test_var_names)
    if (VectorContainsName(_coupled_var_names, test_var_name))
      _trial_var_names.push_back(test_var_name);

  // Otherwise, add it to the set of eliminated variables.
  for (const auto & coupled_var_name : _coupled_var_names)
    if (!VectorContainsName(_test_var_names, coupled_var_name))
      _eliminated_var_names.push_back(coupled_var_name);
}

void
EquationSystem::AddKernel(std::shared_ptr<MFEMKernel> kernel)
{
  const auto & trial_var_name = kernel->getTrialVariableName();
  const auto & test_var_name = kernel->getTestVariableName();
  AddCoupledVariableNameIfMissing(trial_var_name);
  AddTestVariableNameIfMissing(test_var_name);
  // Register new kernels map if not present for the test variable
  if (!_kernels_map.Has(test_var_name))
  {
    auto kernel_field_map =
        std::make_shared<Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMKernel>>>>();
    _kernels_map.Register(test_var_name, std::move(kernel_field_map));
  }
  // Register new kernels map if not present for the test/trial variable pair
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
  const auto & trial_var_name = bc->getTrialVariableName();
  const auto & test_var_name = bc->getTestVariableName();
  AddCoupledVariableNameIfMissing(trial_var_name);
  AddTestVariableNameIfMissing(test_var_name);
  // Register new integrated bc map if not present for the test variable
  if (!_integrated_bc_map.Has(test_var_name))
  {
    auto integrated_bc_field_map = std::make_shared<
        Moose::MFEM::NamedFieldsMap<std::vector<std::shared_ptr<MFEMIntegratedBC>>>>();
    _integrated_bc_map.Register(test_var_name, std::move(integrated_bc_field_map));
  }
  // Register new integrated bc map if not present for the test/trial variable pair
  if (!_integrated_bc_map.Get(test_var_name)->Has(trial_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMIntegratedBC>>>();
    _integrated_bc_map.Get(test_var_name)->Register(trial_var_name, std::move(bcs));
  }
  _integrated_bc_map.GetRef(test_var_name).Get(trial_var_name)->push_back(std::move(bc));
}

void
EquationSystem::AddEssentialBC(std::shared_ptr<MFEMEssentialBC> bc)
{
  const auto & test_var_name = bc->getTestVariableName();
  AddTestVariableNameIfMissing(test_var_name);
  // Register new essential bc map if not present for the test variable
  if (!_essential_bc_map.Has(test_var_name))
  {
    auto bcs = std::make_shared<std::vector<std::shared_ptr<MFEMEssentialBC>>>();
    _essential_bc_map.Register(test_var_name, std::move(bcs));
  }
  _essential_bc_map.GetRef(test_var_name).push_back(std::move(bc));
}

void
EquationSystem::Init(Moose::MFEM::GridFunctions & gridfunctions,
                     Moose::MFEM::ComplexGridFunctions & cmplx_gridfunctions,
                     mfem::AssemblyLevel assembly_level)
{
  _assembly_level = assembly_level;

  if (cmplx_gridfunctions.size())
    mooseError("Complex variables have been created but the executioner numeric type has not been "
               "set to complex. Please set Executioner/numeric_type = complex.");

  // Extract which coupled variables are to be trivially eliminated and which are trial variables
  SetTrialVariableNames();

  for (auto & test_var_name : _test_var_names)
  {
    if (!gridfunctions.Has(test_var_name))
    {
      mooseError("MFEM variable ",
                 test_var_name,
                 " requested by equation system during initialization was "
                 "not found in gridfunctions");
    }
    // Store pointers to test FESpaces
    _test_pfespaces.push_back(gridfunctions.Get(test_var_name)->ParFESpace());
  }

  for (auto & trial_var_name : _trial_var_names)
  {
    if (!gridfunctions.Has(trial_var_name))
    {
      mooseError("MFEM variable ",
                 trial_var_name,
                 " requested by equation system during initialization was "
                 "not found in gridfunctions");
    }
    // Create auxiliary gridfunctions for storing essential constraints from Dirichlet conditions
    _var_ess_constraints.emplace_back(
        std::make_unique<mfem::ParGridFunction>(gridfunctions.Get(trial_var_name)->ParFESpace()));
  }

  // Store pointers to FESpaces of all coupled variables
  for (auto & coupled_var_name : _coupled_var_names)
    _coupled_pfespaces.push_back(gridfunctions.Get(coupled_var_name)->ParFESpace());

  // Store pointers to coupled variable GridFunctions that are to be eliminated prior to forming the
  // jacobian
  for (auto & eliminated_var_name : _eliminated_var_names)
    _eliminated_variables.Register(eliminated_var_name,
                                   gridfunctions.GetShared(eliminated_var_name));

  //Get a reference to the GridFunctions
  _gfuncs = new Moose::MFEM::GridFunctions(gridfunctions);

  //Build the temporary BlockVectors
  _block_true_offsets = new mfem::Array<int>(_trial_var_names.size()+1);
  (*_block_true_offsets)[0] = 0;

  for(unsigned I=0; I<_trial_var_names.size(); I++){
    (*_block_true_offsets)[I+1] = _gfuncs->Get(_trial_var_names.at(I))->ParFESpace()->TrueVSize();
  }
  _block_true_offsets->PartialSum();
  _trueBlockSol.Update(*_block_true_offsets);
  _blockForces.Update(*_block_true_offsets);
  _blockResidual.Update(*_block_true_offsets);
}

void
EquationSystem::ApplyEssentialBC(const std::string & var_name,
                                 mfem::ParGridFunction & trial_gf,
                                 mfem::Array<int> & global_ess_markers)
{
  if (_essential_bc_map.Has(var_name))
  {
    auto & bcs = _essential_bc_map.GetRef(var_name);
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
EquationSystem::ApplyEssentialBCs()
{
  _ess_tdof_lists.resize(_trial_var_names.size());
  for (const auto i : index_range(_trial_var_names))
  {
    const auto & trial_var_name = _trial_var_names.at(i);
    mfem::ParGridFunction & trial_gf = *_var_ess_constraints.at(i);
    trial_gf = 0;
    mfem::Array<int> global_ess_markers(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
    global_ess_markers = 0;
    // Set strongly constrained DoFs of trial_gf on essential boundaries and add markers for all
    // essential boundaries to the global_ess_markers array
    ApplyEssentialBC(trial_var_name, trial_gf, global_ess_markers);
    trial_gf.ParFESpace()->GetEssentialTrueDofs(global_ess_markers, _ess_tdof_lists.at(i));
  }
}

void
EquationSystem::EliminateCoupledVariables()
{
  for (const auto & test_var_name : _test_var_names)
    for (const auto & eliminated_var_name : _eliminated_var_names)
      if (_mblfs.Has(test_var_name) && _mblfs.Get(test_var_name)->Has(eliminated_var_name) &&
          !VectorContainsName(_test_var_names, eliminated_var_name))
      {
        auto & mblf = *_mblfs.Get(test_var_name)->Get(eliminated_var_name);
        mblf.AddMult(*_eliminated_variables.Get(eliminated_var_name), *_lfs.Get(test_var_name), -1);
      }
}

void
EquationSystem::FormLinearSystem(mfem::OperatorHandle & op,
                                 mfem::BlockVector & trueX,
                                 mfem::BlockVector & trueRHS)
{
  mooseAssert(_test_var_names.size() == _trial_var_names.size(),
              "Number of test and trial variables must be the same for block matrix assembly.");

  switch (_assembly_level)
  {
    case mfem::AssemblyLevel::LEGACY:
      FormSystemMatrix(op, trueX, trueRHS);
      break;
    default:
      mooseAssert(_test_var_names.size() == 1,
                  "Non-legacy assembly is only supported for single-variable systems");
      mooseAssert(
          _test_var_names.size() == _trial_var_names.size(),
          "Non-legacy assembly is only supported for single test and trial variable systems");
      FormSystemOperator(op, trueX, trueRHS);
  }
}

void
EquationSystem::FormSystemOperator(mfem::OperatorHandle & op,
                                   mfem::BlockVector & trueX,
                                   mfem::BlockVector & trueRHS)
{
  auto & test_var_name = _test_var_names.at(0);
  mfem::Vector aux_x, aux_rhs;
  mfem::OperatorPtr aux_a;

  auto blf = _blfs.Get(test_var_name);
  blf->FormLinearSystem(_ess_tdof_lists.at(0),
                        *_var_ess_constraints.at(0),
                        *_lfs.Get(test_var_name),
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
EquationSystem::FormSystemMatrix(mfem::OperatorHandle & op,
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
      mfem::ParLinearForm aux_lf(_test_pfespaces.at(i));
      mfem::HypreParMatrix * aux_a = new mfem::HypreParMatrix;

      if (test_var_name == trial_var_name)
      {
        mooseAssert(i == j, "Trial and test variables must have the same ordering.");
        auto blf = _blfs.Get(test_var_name);
        blf->FormLinearSystem(_ess_tdof_lists.at(j),
                              *_var_ess_constraints.at(j),
                              *_lfs.Get(test_var_name),
                              *aux_a,
                              aux_x,
                              aux_rhs,
                              /*copy_interior=*/true);
        trueX.GetBlock(j) = aux_x;
      }
      else if (_mblfs.Has(test_var_name) && _mblfs.Get(test_var_name)->Has(trial_var_name))
      {
        auto mblf = _mblfs.Get(test_var_name)->Get(trial_var_name);
        mblf->FormRectangularLinearSystem(_ess_tdof_lists.at(j),
                                          _ess_tdof_lists.at(i),
                                          *_var_ess_constraints.at(j),
                                          aux_lf = 0,
                                          *aux_a,
                                          aux_x,
                                          aux_rhs);
      }
      else
        continue;

      trueRHS.GetBlock(i) += aux_rhs;
      _h_blocks(i, j) = aux_a;
    }
  }
  // Sync memory
  trueX.SyncFromBlocks();
  trueRHS.SyncFromBlocks();

  // Create monolithic matrix
  op.Reset(mfem::HypreParMatrixFromBlocks(_h_blocks));
}

void EquationSystem::ReassembleJacobian(mfem::BlockVector & x, mfem::BlockVector & rhs)
{
  //Reassemble all the Forms
  for (const auto I : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(I);
    _blfs.GetShared(test_var_name)->Update();
    _blfs.GetShared(test_var_name)->Assemble();
    if (_mblfs.Has(test_var_name)){
      for (const auto J : index_range(_coupled_var_names))
      {
        auto coupled_var_name = _coupled_var_names.at(J);
        if (_mblfs.Get(test_var_name)->Has(coupled_var_name))
        {
          _mblfs.GetShared(test_var_name)->GetShared(coupled_var_name)->Update();
          _mblfs.GetShared(test_var_name)->GetShared(coupled_var_name)->Assemble();
        }
      }
    }
  }

  //Form the system matrix
  //This uses dummy arguments
  //for the vectors
  FormLinearSystem(_jacobian, x, rhs);
};

void
EquationSystem::BuildJacobian(mfem::BlockVector & trueX, mfem::BlockVector & trueRHS)
{
  height = trueX.Size();
  width = trueRHS.Size();
  FormLinearSystem(_jacobian, trueX, trueRHS);
}

void
EquationSystem::UpdateJacobian() const
{

  for (unsigned int i = 0; i < _test_var_names.size(); i++)
  {
    auto & test_var_name = _test_var_names.at(i);
    auto blf = _blfs.Get(test_var_name);
    blf->Update();
    blf->Assemble();
  }

  // Form off-diagonal blocks
  for (unsigned int i = 0; i < _test_var_names.size(); i++)
  {
    auto test_var_name = _test_var_names.at(i);
    for (unsigned int j = 0; j < _test_var_names.size(); j++)
    {
      auto trial_var_name = _test_var_names.at(j);
      if (_mblfs.Has(test_var_name) && _mblfs.Get(test_var_name)->Has(trial_var_name))
      {
        auto mblf = _mblfs.Get(test_var_name)->Get(trial_var_name);
        mblf->Update();
        mblf->Assemble();
      }
    }
  }
}

void CopyVec(const mfem::Vector & x, mfem::Vector & y){y = x;};

void
EquationSystem::Mult(const mfem::Vector & sol, mfem::Vector & residual) const
{
  static_cast<mfem::Vector &>(_trueBlockSol) = sol;
  for (unsigned int i = 0; i < _trial_var_names.size(); i++)
  {
    auto & trial_var_name = _trial_var_names.at(i);
    _trueBlockSol.GetBlock(i).SyncAliasMemory(_trueBlockSol);
    _gfuncs->Get(trial_var_name)->Distribute(&(_trueBlockSol.GetBlock(i)));
  }

  if (_non_linear)
  {
    _blockResidual = 0.0;
    UpdateJacobian();

    for (unsigned int i = 0; i < _test_var_names.size(); i++)
    {
      auto & test_var_name = _test_var_names.at(i);
      int offset = _blockResidual.GetBlock(i).Size();
      mfem::Vector b(offset);

      auto lf = _lfs.GetShared(test_var_name);
      lf->Assemble();
      lf->ParallelAssemble(b);
      b.SyncAliasMemory(b);

      auto nlf = _nlAs.GetShared(test_var_name);
      nlf->Assemble();
      nlf->ParallelAssemble(_blockResidual.GetBlock(i));

      _blockResidual.GetBlock(i) -= b;
      _blockResidual.GetBlock(i) *= -1;

      _blockResidual.GetBlock(i).SetSubVector(_ess_tdof_lists.at(i), 0.0);
      _blockResidual.GetBlock(i).SyncAliasMemory(_blockResidual);
    }

    residual = static_cast<mfem::Vector &>(_blockResidual);
    const_cast<EquationSystem *>(this)->FormLinearSystem(_jacobian, _trueBlockSol, _blockResidual);
    residual *= -1.0;
  }
  else
  {
    residual = 0.0;
    _jacobian->Mult(sol, residual);
  }

  sol.HostRead();
  residual.HostRead();
}



mfem::Operator &
EquationSystem::GetGradient(const mfem::Vector & x) const
{
  return *_jacobian;
}

void
EquationSystem::RecoverFEMSolution(mfem::BlockVector & trueX,
                                   Moose::MFEM::GridFunctions & gridfunctions,
                                   Moose::MFEM::ComplexGridFunctions & /*cmplx_gridfunctions*/)
{
  for (const auto i : index_range(_trial_var_names))
    gridfunctions.Get(_trial_var_names.at(i))->Distribute(&(trueX.GetBlock(i)));
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

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto lf = _lfs.GetShared(test_var_name);
    ApplyDomainLFIntegrators(test_var_name, lf, _kernels_map);
    ApplyBoundaryLFIntegrators(test_var_name, lf, _integrated_bc_map);
    lf->Assemble();
  }

  // Apply essential boundary conditions
  ApplyEssentialBCs();

  // Eliminate trivially eliminated variables by subtracting contributions from linear forms
  EliminateCoupledVariables();
}

void
EquationSystem::BuildNonLinearActionForms()
{
  // Register non-linear Action forms
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    _nlAs.Register(test_var_name, std::make_shared<mfem::ParLinearForm>(_test_pfespaces.at(i)));
    _nlAs.GetRef(test_var_name) = 0.0;
  }

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto nlA = _nlAs.GetShared(test_var_name);
    ApplyDomainNLAFIntegrators(test_var_name, nlA, _kernels_map);
    ApplyBoundaryNLAFIntegrators(test_var_name, nlA, _integrated_bc_map);
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
    blf->SetAssemblyLevel(_assembly_level);
    ApplyBoundaryBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, blf, _integrated_bc_map);
    ApplyDomainBLFIntegrators<mfem::ParBilinearForm>(
        test_var_name, test_var_name, blf, _kernels_map);
    // Assemble
    blf->Assemble();
  }
}

void
EquationSystem::BuildMixedBilinearForms()
{
  // Register mixed bilinear forms. Note that not all combinations may
  // have a kernel.

  // Create mblf for each test/coupled variable pair with an added kernel.
  // Mixed bilinear forms with coupled variables that are not trial variables are
  // associated with contributions from eliminated variables.
  for (const auto i : index_range(_test_var_names))
  {
    auto test_var_name = _test_var_names.at(i);
    auto test_mblfs = std::make_shared<Moose::MFEM::NamedFieldsMap<mfem::ParMixedBilinearForm>>();
    for (const auto j : index_range(_coupled_var_names))
    {
      const auto & coupled_var_name = _coupled_var_names.at(j);
      auto mblf = std::make_shared<mfem::ParMixedBilinearForm>(_coupled_pfespaces.at(j),
                                                               _test_pfespaces.at(i));
      // Register MixedBilinearForm if kernels exist for it, and assemble kernels
      if (_kernels_map.Has(test_var_name) &&
          _kernels_map.Get(test_var_name)->Has(coupled_var_name) &&
          test_var_name != coupled_var_name)
      {
        mblf->SetAssemblyLevel(_assembly_level);
        // Apply all mixed kernels with this test/trial pair
        ApplyDomainBLFIntegrators<mfem::ParMixedBilinearForm>(
            coupled_var_name, test_var_name, mblf, _kernels_map);
        // Assemble mixed bilinear forms
        mblf->Assemble();
        // Register mixed bilinear forms associated with a single trial variable
        // for the current test variable
        test_mblfs->Register(coupled_var_name, mblf);
      }
    }
    // Register all mixed bilinear form sets associated with a single test variable
    _mblfs.Register(test_var_name, test_mblfs);
  }
}

void
EquationSystem::BuildEquationSystem()
{
  BuildBilinearForms();
  BuildMixedBilinearForms();
  BuildLinearForms();
  BuildNonLinearActionForms();
}

} // namespace Moose::MFEM

#endif
