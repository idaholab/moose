#include "scaled_vector_gridfunction_aux.hpp"

#include <utility>

namespace hephaestus
{
// result = ax+by vector gridfunction aux
// y should be in same FESpace as result
// shift_gf_name?
// b_Const
ScaledVectorGridFunctionAux::ScaledVectorGridFunctionAux(std::string input_gf_name,
                                                         std::string scaled_gf_name,
                                                         std::string coef_name,
                                                         const double & aConst,
                                                         const double & bConst,
                                                         std::string shift_gf_name,
                                                         hephaestus::InputParameters solver_options)
  : _input_gf_name(std::move(input_gf_name)),
    _scaled_gf_name(std::move(scaled_gf_name)),
    _shift_gf_name(std::move(shift_gf_name)),
    _coef_name(std::move(coef_name)),
    _a_const(aConst),
    _b_const(bConst),
    _solver_options(std::move(solver_options)),

    _a(nullptr),
    _a_mixed(nullptr),
    _a_mat(nullptr),
    _mixed_mat(nullptr),
    _solver(nullptr)
{
}

void
ScaledVectorGridFunctionAux::Init(const hephaestus::GridFunctions & gridfunctions,
                                  hephaestus::Coefficients & coefficients)
{
  _input_gf = gridfunctions.Get(_input_gf_name);
  _scaled_gf = gridfunctions.Get(_scaled_gf_name);
  _coef = coefficients._scalars.Get(_coef_name);

  if (!_shift_gf_name.empty())
  {
    _shift_gf = gridfunctions.Get(_shift_gf_name);
    // TODO - add check that shift_gf is on same FESpace as scaled_gf
  }
  _test_fes = _scaled_gf->ParFESpace();
  _trial_fes = _input_gf->ParFESpace();
  BuildBilinearForm();
  BuildMixedBilinearForm();
  _a_mat = std::unique_ptr<mfem::HypreParMatrix>(_a->ParallelAssemble());

  _solver = std::make_unique<hephaestus::DefaultJacobiPCGSolver>(_solver_options, *_a_mat);
}

void
ScaledVectorGridFunctionAux::BuildBilinearForm()
{
  _a = std::make_unique<mfem::ParBilinearForm>(_test_fes);
  _a->AddDomainIntegrator(new mfem::VectorFEMassIntegrator());
  _a->Assemble();
  _a->Finalize();
}

void
ScaledVectorGridFunctionAux::BuildMixedBilinearForm()
{
  _a_mixed = std::make_unique<mfem::ParMixedBilinearForm>(_trial_fes, _test_fes);
  _a_mixed->AddDomainIntegrator(new mfem::MixedVectorMassIntegrator(*_coef));
  _a_mixed->Assemble();
  _a_mixed->Finalize();
}

void
ScaledVectorGridFunctionAux::Solve(double t)
{
  mfem::Vector b(_test_fes->GetTrueVSize());  // Linear form true DOFs
  mfem::Vector x(_test_fes->GetTrueVSize());  // H(Div) gridfunction true DOFs
  mfem::Vector p(_trial_fes->GetTrueVSize()); // H(Curl) gridfunction true DOFs
  b = 0.0;
  _input_gf->GetTrueDofs(p);

  // Reassemble in case coef has changed
  _a_mixed->Update();
  _a_mixed->Assemble();
  _a_mixed->Finalize();

  _mixed_mat = std::unique_ptr<mfem::HypreParMatrix>(_a_mixed->ParallelAssemble());
  _mixed_mat->AddMult(p, b, _a_const);

  x = 0.0;
  _solver->Mult(b, x);
  _scaled_gf->SetFromTrueDofs(x);
  if (_shift_gf != nullptr)
  {
    _scaled_gf->Add(_b_const, *_shift_gf);
  }
}

} // namespace hephaestus
