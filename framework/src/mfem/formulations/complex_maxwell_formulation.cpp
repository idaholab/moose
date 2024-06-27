#include "complex_maxwell_formulation.hpp"

#include <utility>

namespace hephaestus
{

ComplexMaxwellFormulation::ComplexMaxwellFormulation(std::string alpha_coef_name,
                                                     std::string beta_coef_name,
                                                     std::string zeta_coef_name,
                                                     std::string frequency_coef_name,
                                                     std::string h_curl_var_complex_name,
                                                     std::string h_curl_var_real_name,
                                                     std::string h_curl_var_imag_name)
  : _alpha_coef_name(std::move(alpha_coef_name)),
    _beta_coef_name(std::move(beta_coef_name)),
    _zeta_coef_name(std::move(zeta_coef_name)),
    _frequency_coef_name(std::move(frequency_coef_name)),
    _h_curl_var_complex_name(std::move(h_curl_var_complex_name)),
    _h_curl_var_real_name(std::move(h_curl_var_real_name)),
    _h_curl_var_imag_name(std::move(h_curl_var_imag_name)),
    _mass_coef_name(std::string("maxwell_mass")),
    _loss_coef_name(std::string("maxwell_loss"))
{
}

void
ComplexMaxwellFormulation::ConstructJacobianSolver()
{
  ConstructJacobianSolverWithOptions(SolverType::SUPER_LU);
}

void
ComplexMaxwellFormulation::ConstructOperator()
{
  auto new_operator = std::make_unique<hephaestus::ComplexMaxwellOperator>(*GetProblem(),
                                                                           _h_curl_var_complex_name,
                                                                           _h_curl_var_real_name,
                                                                           _h_curl_var_imag_name,
                                                                           _alpha_coef_name,
                                                                           _mass_coef_name,
                                                                           _loss_coef_name);

  GetProblem()->SetOperator(std::move(new_operator));
}

void
ComplexMaxwellFormulation::RegisterGridFunctions()
{
  int & myid = GetProblem()->_myid;
  hephaestus::GridFunctions & gridfunctions = GetProblem()->_gridfunctions;
  hephaestus::FESpaces & fespaces = GetProblem()->_fespaces;

  // Register default ParGridFunctions of state gridfunctions if not provided
  if (!gridfunctions.Has(_h_curl_var_real_name))
  {
    if (myid == 0)
    {
      MFEM_WARNING(_h_curl_var_real_name << " not found in gridfunctions: building "
                                            "gridfunction from defaults");
    }
    AddFESpace(std::string("_HCurlFESpace"), std::string("ND_3D_P1"));
    AddGridFunction(_h_curl_var_real_name, std::string("_HCurlFESpace"));
    AddGridFunction(_h_curl_var_imag_name, std::string("_HCurlFESpace"));
  }
}

void
ComplexMaxwellFormulation::RegisterCoefficients()
{
  hephaestus::Coefficients & coefficients = GetProblem()->_coefficients;

  if (!coefficients._scalars.Has(_frequency_coef_name))
  {
    MFEM_ABORT(_frequency_coef_name + " coefficient not found.");
  }
  if (!coefficients._scalars.Has("magnetic_permeability"))
  {
    MFEM_ABORT("Magnetic permeability coefficient not found.");
  }
  if (!coefficients._scalars.Has(_beta_coef_name))
  {
    MFEM_ABORT(_beta_coef_name + " coefficient not found.");
  }
  if (!coefficients._scalars.Has(_zeta_coef_name))
  {
    MFEM_ABORT(_zeta_coef_name + " coefficient not found.");
  }

  _freq_coef = coefficients._scalars.Get<mfem::ConstantCoefficient>(_frequency_coef_name);

  // define transformed
  coefficients._scalars.Register(
      "_angular_frequency",
      std::make_shared<mfem::ConstantCoefficient>(2.0 * M_PI * _freq_coef->constant));
  coefficients._scalars.Register(
      "_neg_angular_frequency",
      std::make_shared<mfem::ConstantCoefficient>(-2.0 * M_PI * _freq_coef->constant));
  coefficients._scalars.Register(
      "_angular_frequency_sq",
      std::make_shared<mfem::ConstantCoefficient>(pow(2.0 * M_PI * _freq_coef->constant, 2)));
  coefficients._scalars.Register(
      "_neg_angular_frequency_sq",
      std::make_shared<mfem::ConstantCoefficient>(-pow(2.0 * M_PI * _freq_coef->constant, 2)));

  coefficients._scalars.Register("_inv_angular_frequency",
                                 std::make_shared<mfem::RatioCoefficient>(
                                     1.0, coefficients._scalars.GetRef("_angular_frequency")));

  coefficients._scalars.Register(_mass_coef_name,
                                 std::make_shared<mfem::TransformedCoefficient>(
                                     coefficients._scalars.Get("_neg_angular_frequency_sq"),
                                     coefficients._scalars.Get(_zeta_coef_name),
                                     prodFunc));

  coefficients._scalars.Register(_loss_coef_name,
                                 std::make_shared<mfem::TransformedCoefficient>(
                                     coefficients._scalars.Get("_angular_frequency"),
                                     coefficients._scalars.Get(_beta_coef_name),
                                     prodFunc));

  coefficients._scalars.Register(
      _alpha_coef_name,
      std::make_shared<mfem::TransformedCoefficient>(
          &_one_coef, coefficients._scalars.Get("magnetic_permeability"), fracFunc));
}

ComplexMaxwellOperator::ComplexMaxwellOperator(hephaestus::Problem & problem,
                                               std::string h_curl_var_complex_name,
                                               std::string h_curl_var_real_name,
                                               std::string h_curl_var_imag_name,
                                               std::string stiffness_coef_name,
                                               std::string mass_coef_name,
                                               std::string loss_coef_name)
  : ProblemOperator(problem),
    _h_curl_var_complex_name(std::move(h_curl_var_complex_name)),
    _h_curl_var_real_name(std::move(h_curl_var_real_name)),
    _h_curl_var_imag_name(std::move(h_curl_var_imag_name)),
    _stiffness_coef_name(std::move(stiffness_coef_name)),
    _mass_coef_name(std::move(mass_coef_name)),
    _loss_coef_name(std::move(loss_coef_name))
{
}

void
ComplexMaxwellOperator::SetGridFunctions()
{
  _trial_var_names.push_back(_h_curl_var_real_name);
  _trial_var_names.push_back(_h_curl_var_imag_name);

  ProblemOperator::SetGridFunctions();

  _u = std::make_unique<mfem::ParComplexGridFunction>(_trial_variables.at(0)->ParFESpace());
  *_u = std::complex(0.0, 0.0);
};

void
ComplexMaxwellOperator::Init(mfem::Vector & X)
{
  ProblemOperator::Init(X);

  _stiff_coef = _problem._coefficients._scalars.Get(_stiffness_coef_name);

  if (_problem._coefficients._scalars.Has(_mass_coef_name))
    _mass_coef = _problem._coefficients._scalars.Get(_mass_coef_name);
  if (_problem._coefficients._scalars.Has(_loss_coef_name))
    _loss_coef = _problem._coefficients._scalars.Get(_loss_coef_name);
}

void
ComplexMaxwellOperator::Solve(mfem::Vector & X)
{
  mfem::OperatorHandle jac;
  mfem::Vector u, rhs;
  mfem::OperatorHandle pc_op;

  mfem::Vector zero_vec(3);
  zero_vec = 0.0;
  mfem::VectorConstantCoefficient zero_coef(zero_vec);

  mfem::ParSesquilinearForm sqlf(_u->ParFESpace(), _conv);
  sqlf.AddDomainIntegrator(new mfem::CurlCurlIntegrator(*_stiff_coef), nullptr);
  if (_mass_coef)
  {
    sqlf.AddDomainIntegrator(new mfem::VectorFEMassIntegrator(*_mass_coef), nullptr);
  }
  if (_loss_coef)
  {
    sqlf.AddDomainIntegrator(nullptr, new mfem::VectorFEMassIntegrator(*_loss_coef));
  }

  mfem::ParLinearForm lf_real(_u->ParFESpace());
  mfem::ParLinearForm lf_imag(_u->ParFESpace());
  lf_real = 0.0;
  lf_imag = 0.0;

  _problem._sources.Apply(&lf_real);

  mfem::ParComplexLinearForm lf(_u->ParFESpace(), _conv);
  _problem._bc_map.ApplyEssentialBCs(
      _h_curl_var_complex_name, _ess_bdr_tdofs, *_u, _problem._pmesh.get());
  _problem._bc_map.ApplyIntegratedBCs(_h_curl_var_complex_name, lf, _problem._pmesh.get());
  _problem._bc_map.ApplyIntegratedBCs(_h_curl_var_complex_name, sqlf, _problem._pmesh.get());

  sqlf.Assemble();
  sqlf.Finalize();

  lf.Assemble();
  lf.real() += lf_real;
  lf.imag() += lf_imag;

  sqlf.FormLinearSystem(_ess_bdr_tdofs, *_u, lf, jac, u, rhs);

  auto * jac_z = jac.As<mfem::ComplexHypreParMatrix>()->GetSystemMatrix();

  _problem._jacobian_solver->SetOperator(*jac_z);
  _problem._jacobian_solver->Mult(rhs, u);
  sqlf.RecoverFEMSolution(u, lf, *_u);

  _problem._gridfunctions.GetRef(_trial_var_names.at(0)) = _u->real();
  _problem._gridfunctions.GetRef(_trial_var_names.at(1)) = _u->imag();
}

} // namespace hephaestus
