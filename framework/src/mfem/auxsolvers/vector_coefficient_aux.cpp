#include "vector_coefficient_aux.hpp"

#include <utility>

namespace hephaestus
{

VectorCoefficientAux::VectorCoefficientAux(std::string gf_name,
                                           std::string vec_coef_name,
                                           hephaestus::InputParameters solver_options)
  : _gf_name(std::move(gf_name)),
    _vec_coef_name(std::move(vec_coef_name)),
    _solver_options(std::move(solver_options))
{
}

void
VectorCoefficientAux::Init(const hephaestus::GridFunctions & gridfunctions,
                           hephaestus::Coefficients & coefficients)
{
  _gf = gridfunctions.Get(_gf_name);
  _vec_coef = coefficients._vectors.Get(_vec_coef_name);
  _test_fes = _gf->ParFESpace();

  BuildBilinearForm();
  BuildLinearForm();
  _a_mat = std::unique_ptr<mfem::HypreParMatrix>(_a->ParallelAssemble());
  _solver = std::make_unique<hephaestus::DefaultJacobiPCGSolver>(_solver_options, *_a_mat);
}

void
VectorCoefficientAux::BuildBilinearForm()
{
  _a = std::make_unique<mfem::ParBilinearForm>(_test_fes);
  if (_test_fes->FEColl()->GetRangeType(3) == mfem::FiniteElement::SCALAR)
  {
    _a->AddDomainIntegrator(new mfem::VectorMassIntegrator());
  }
  else
  {
    _a->AddDomainIntegrator(new mfem::VectorFEMassIntegrator());
  }
  _a->Assemble();
  _a->Finalize();
}

void
VectorCoefficientAux::BuildLinearForm()
{
  _b = std::make_unique<mfem::ParLinearForm>(_test_fes);
  if (_test_fes->FEColl()->GetRangeType(3) == mfem::FiniteElement::SCALAR)
  {
    _b->AddDomainIntegrator(new mfem::VectorDomainLFIntegrator(*_vec_coef));
  }
  else
  {
    _b->AddDomainIntegrator(new mfem::VectorFEDomainLFIntegrator(*_vec_coef));
  }
  _b->Assemble();
}

void
VectorCoefficientAux::Solve(double t)
{
  mfem::Vector x(_test_fes->GetTrueVSize()); // Gridfunction true DOFs
  _gf->ProjectCoefficient(*_vec_coef);       // Initial condition
  _gf->GetTrueDofs(x);

  // Reassemble in case coef has changed
  _b->Update();
  _b->Assemble();

  _solver->Mult(*_b, x);
  _gf->SetFromTrueDofs(x);
}

} // namespace hephaestus
