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
    _cxs.emplace_back(
        std::make_unique<mfem::ParComplexGridFunction>(cpx_gridfunctions.Get(test_var_name)->ParFESpace()));
    _cdxdts.emplace_back(
        std::make_unique<mfem::ParComplexGridFunction>(cpx_gridfunctions.Get(test_var_name)->ParFESpace()));
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
    _clfs.Register(test_var_name, std::make_shared<mfem::ParComplexLinearForm>(_test_pfespaces.at(i)));
    _clfs.GetRef(test_var_name) = 0.0;
  }
  // Apply boundary conditions
  ApplyEssentialBCs();

  for (auto & test_var_name : _test_var_names)
  {
    // Apply kernels
    auto clf = _clfs.GetShared(test_var_name);
    ApplyDomainLFIntegrators(test_var_name, clf, _kernels_map);
    ApplyBoundaryLFIntegrators(test_var_name, clf, _integrated_bc_map);
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
    _slfs.Register(test_var_name, std::make_shared<mfem::ParSesquilinearForm>(_test_pfespaces.at(i)));

    // Apply kernels
    auto slf = _slfs.GetShared(test_var_name);
    slf->SetAssemblyLevel(_assembly_level);
    ApplyBoundaryBLFIntegrators<mfem::ParSesquilinearForm>(
        test_var_name, test_var_name, slf, _integrated_bc_map);
    ApplyDomainBLFIntegrators<mfem::ParSesquilinearForm>(
        test_var_name, test_var_name, slf, _kernels_map);
    // Assemble
    slf->Assemble();
  }
}


}

#endif
