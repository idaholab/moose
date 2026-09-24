#ifdef MOOSE_MFEM_ENABLED

#include "gtest/gtest.h"
#include "NLCurlCurlIntegrator.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"
#include "SumOperatorExtension.h"

TEST(CheckData, NLCurlCurlIntegratorJacobianMatchesAnalyticLinearization)
{
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(1, 1, 1, mfem::Element::TETRAHEDRON, 1.0, 1.0);
  mfem::ND_FECollection fec(1, mesh.Dimension());
  mfem::FiniteElementSpace fespace(&mesh, &fec);

  ASSERT_EQ(fespace.GetNE(), 6);

  mfem::GridFunction gf(&fespace);
  gf = 0.0;

  mfem::Array<int> vdofs;
  fespace.GetElementVDofs(0, vdofs);

  mfem::Vector elfun(vdofs.Size());
  for (int i = 0; i < elfun.Size(); ++i)
    elfun(i) = 1.0 + 0.5 * i;
  gf.SetSubVector(vdofs, elfun);

  mfem::CurlGridFunctionCoefficient curl_gf_coeff(&gf);
  MFEMVectorMagnitudeCoefficient curl_u_norm_coeff(curl_gf_coeff);
  mfem::TransformedCoefficient k_coeff(&curl_u_norm_coeff, [](double u) { return u * u; });
  mfem::TransformedCoefficient curlu_dk_dcurlu_coeff(&curl_u_norm_coeff,
                                                     [](double u) { return 2.0 * u * u; });
  // This unit test is for the element level assembly. The constructor needs a function
  // to represent dk/ds for the partial assembly route. Since we don't need to test that
  // here, we just pass a dummy function in.
  mfem::TransformedCoefficient dk_ds_coeff(&curl_u_norm_coeff, [](double /*u*/) { return 2; });

  const auto & ir = mfem::IntRules.Get(fespace.GetFE(0)->GetGeomType(), 2);

  Moose::MFEM::NLCurlCurlIntegrator integ(
      k_coeff, curlu_dk_dcurlu_coeff, dk_ds_coeff, curl_gf_coeff, 1e-32, &ir);

  const auto & el = *fespace.GetFE(0);
  auto & T = *mesh.GetElementTransformation(0);

  mfem::DenseMatrix jacobian_numeric;
  integ.AssembleElementGrad(el, T, elfun, jacobian_numeric);

  mfem::DenseMatrix jacobian_expected(el.GetDof());
  jacobian_expected = 0.0;

  mfem::DenseMatrix curlshape(el.GetDof(), el.GetCurlDim());
  mfem::Vector curl_u(el.GetCurlDim());
  mfem::Vector curl_u_hat(el.GetCurlDim());

  for (int qp = 0; qp < ir.GetNPoints(); ++qp)
  {
    const auto & ip = ir.IntPoint(qp);
    T.SetIntPoint(&ip);

    el.CalcPhysCurlShape(T, curlshape);
    curlshape.MultTranspose(elfun, curl_u);

    const mfem::real_t curl_u_norm = curl_u.Norml2();
    if (curl_u_norm > 1e-32)
    {
      curl_u_hat = curl_u;
      curl_u_hat /= curl_u_norm;
    }
    else
      curl_u_hat = 0.0;

    const mfem::real_t weight = ip.weight * T.Weight();
    const mfem::real_t k = curl_u_norm * curl_u_norm;
    const mfem::real_t curlu_dk_dcurlu = 2.0 * curl_u_norm * curl_u_norm;

    for (int i = 0; i < el.GetDof(); ++i)
      for (int j = 0; j < el.GetDof(); ++j)
      {
        mfem::real_t curl_trial_dot_curl_test = 0.0;
        mfem::real_t curl_hat_dot_curl_test = 0.0;
        mfem::real_t curl_hat_dot_curl_trial = 0.0;
        for (int d = 0; d < el.GetCurlDim(); ++d)
        {
          curl_trial_dot_curl_test += curlshape(j, d) * curlshape(i, d);
          curl_hat_dot_curl_test += curl_u_hat(d) * curlshape(i, d);
          curl_hat_dot_curl_trial += curl_u_hat(d) * curlshape(j, d);
        }

        jacobian_expected(i, j) +=
            weight * (k * curl_trial_dot_curl_test +
                      curlu_dk_dcurlu * curl_hat_dot_curl_test * curl_hat_dot_curl_trial);
      }
  }

  jacobian_numeric -= jacobian_expected;
  EXPECT_NEAR(jacobian_numeric.MaxMaxNorm(), 0.0, 1e-12);
}

TEST(CheckData, NLCurlCurlIntegratorPartialAssemblyMatchesLegacy)
{
  // Partial assembly requires tensor-product elements - PreAssemblySetup rejects anything that
  // is not a VectorTensorFiniteElement - so this uses hexes rather than the tets above.
  mfem::Mesh mesh = mfem::Mesh::MakeCartesian3D(2, 2, 2, mfem::Element::HEXAHEDRON);
  mfem::ND_FECollection fec(1, mesh.Dimension());
  mfem::FiniteElementSpace fespace(&mesh, &fec);

  // A state with a spatially varying curl, so k(|curl u|) is not effectively constant and the
  // rank-one term in the gradient is actually exercised.
  mfem::GridFunction gf(&fespace);
  mfem::VectorFunctionCoefficient seed(3,
                                       [](const mfem::Vector & p, mfem::Vector & v)
                                       {
                                         v(0) = std::sin(p(1));
                                         v(1) = std::cos(p(2));
                                         v(2) = std::sin(p(0));
                                       });
  gf.ProjectCoefficient(seed);

  // k(s) = 1 + s^2, so k >= 1 everywhere and the curl-curl block stays non-degenerate.
  // Then s k'(s) = 2 s^2 and k'(s)/s = 2.
  mfem::CurlGridFunctionCoefficient curl_gf_coeff(&gf);
  MFEMVectorMagnitudeCoefficient curl_u_norm_coeff(curl_gf_coeff);
  mfem::TransformedCoefficient k_coeff(&curl_u_norm_coeff, [](double s) { return 1.0 + s * s; });
  mfem::TransformedCoefficient curlu_dk_dcurlu_coeff(&curl_u_norm_coeff,
                                                     [](double s) { return 2.0 * s * s; });
  mfem::ConstantCoefficient dk_ds_over_s_coeff(2.0);

  // Both forms must integrate on the same rule, or this measures quadrature rather than
  // assembly: the legacy path uses order 2p, while AssemblePA defaults to
  // MassIntegrator::GetRule, which is 2p + Trans.OrderW().
  const auto & ir = mfem::IntRules.Get(mfem::Geometry::CUBE, 2 * fec.GetOrder());

  auto add_integrator = [&](mfem::NonlinearForm & form)
  {
    form.AddDomainIntegrator(new Moose::MFEM::NLCurlCurlIntegrator(
        k_coeff, curlu_dk_dcurlu_coeff, dk_ds_over_s_coeff, curl_gf_coeff, 1e-32, &ir));
  };

  mfem::NonlinearForm legacy(&fespace);
  add_integrator(legacy);
  legacy.SetAssemblyLevel(mfem::AssemblyLevel::LEGACY);
  legacy.Setup();

  mfem::NonlinearForm partial(&fespace);
  add_integrator(partial);
  partial.SetAssemblyLevel(mfem::AssemblyLevel::PARTIAL);
  partial.Setup();

  mfem::Vector x(fespace.GetTrueVSize());
  x = gf;

  // Residual action: AssemblePA and AddMultPA against the legacy element loop.
  mfem::Vector r_legacy(x.Size()), r_partial(x.Size());
  legacy.Mult(x, r_legacy);
  partial.Mult(x, r_partial);
  ASSERT_GT(r_legacy.Norml2(), 0.0);
  mfem::Vector r_diff(r_legacy);
  r_diff -= r_partial;
  EXPECT_LT(r_diff.Norml2(), 1e-10 * r_legacy.Norml2());

  // Gradient: compared column by column rather than by a few random probes. The partially
  // assembled gradient is never formed as a matrix, so each column is recovered by applying
  // it to a unit vector; the mesh is small enough that this is cheap and it catches errors
  // that random directions can miss.
  mfem::Operator & g_legacy = legacy.GetGradient(x);
  mfem::Operator & g_partial = partial.GetGradient(x);

  mfem::Vector e(x.Size()), col_legacy(x.Size()), col_partial(x.Size());
  mfem::real_t max_entry = 0.0, max_diff = 0.0;
  for (const auto j : make_range(x.Size()))
  {
    e = 0.0;
    e(j) = 1.0;
    g_legacy.Mult(e, col_legacy);
    g_partial.Mult(e, col_partial);
    max_entry = std::max(max_entry, col_legacy.Normlinf());
    col_partial -= col_legacy;
    max_diff = std::max(max_diff, col_partial.Normlinf());
  }
  ASSERT_GT(max_entry, 0.0);
  EXPECT_LT(max_diff, 1e-10 * max_entry);
}

TEST(CheckData, SumOperatorExtensionDiagonalMatchesItsAction)
{
  mfem::Mesh serial = mfem::Mesh::MakeCartesian3D(2, 2, 2, mfem::Element::HEXAHEDRON);
  mfem::ParMesh mesh(MPI_COMM_WORLD, serial);
  mfem::ND_FECollection fec(1, mesh.Dimension());
  mfem::ParFiniteElementSpace fespace(&mesh, &fec);

  mfem::ParGridFunction gf(&fespace);
  mfem::VectorFunctionCoefficient seed(3,
                                       [](const mfem::Vector & p, mfem::Vector & v)
                                       {
                                         v(0) = std::sin(p(1));
                                         v(1) = std::cos(p(2));
                                         v(2) = std::sin(p(0));
                                       });
  gf.ProjectCoefficient(seed);

  // k(s) = 1 + s^2, so s k'(s) = 2 s^2 and k'(s)/s = 2.
  mfem::CurlGridFunctionCoefficient curl_gf_coeff(&gf);
  MFEMVectorMagnitudeCoefficient curl_u_norm_coeff(curl_gf_coeff);
  mfem::TransformedCoefficient k_coeff(&curl_u_norm_coeff, [](double s) { return 1.0 + s * s; });
  mfem::TransformedCoefficient curlu_dk_dcurlu_coeff(&curl_u_norm_coeff,
                                                     [](double s) { return 2.0 * s * s; });
  mfem::ConstantCoefficient dk_ds_over_s_coeff(2.0), one(1.0);

  mfem::Array<int> ess_bdr(mesh.bdr_attributes.Max()), ess_tdofs;
  ess_bdr = 1;
  fespace.GetEssentialTrueDofs(ess_bdr, ess_tdofs);
  ASSERT_GT(ess_tdofs.Size(), 0);

  // A: the nonlinear form's partially assembled gradient, carrying the DIAG_ZERO policy that
  // EquationSystem::GetGradient sets on it.
  mfem::ParNonlinearForm nlf(&fespace);
  nlf.AddDomainIntegrator(new Moose::MFEM::NLCurlCurlIntegrator(
      k_coeff, curlu_dk_dcurlu_coeff, dk_ds_over_s_coeff, curl_gf_coeff, 1e-32));
  nlf.SetEssentialTrueDofs(ess_tdofs);
  nlf.SetAssemblyLevel(mfem::AssemblyLevel::PARTIAL);
  nlf.Setup();

  mfem::Vector x(fespace.GetTrueVSize());
  gf.GetTrueDofs(x);

  auto * const nlf_grad = dynamic_cast<mfem::ConstrainedOperator *>(&nlf.GetGradient(x));
  ASSERT_NE(nlf_grad, nullptr);
  nlf_grad->SetDiagonalPolicy(mfem::Operator::DIAG_ZERO);

  // B: the partially assembled linear operator, which picks up DIAG_ONE on the essential rows
  // from FormSystemMatrix - the same elimination EquationSystem::FormSystemOperator relies on.
  mfem::ParBilinearForm blf(&fespace);
  blf.AddDomainIntegrator(new mfem::VectorFEMassIntegrator(one));
  blf.SetAssemblyLevel(mfem::AssemblyLevel::PARTIAL);
  blf.Assemble();
  mfem::OperatorHandle linear_op;
  blf.FormSystemMatrix(ess_tdofs, linear_op);

  Moose::MFEM::SumOperatorExtension sum(nlf_grad, linear_op.Ptr(), &nlf);

  mfem::Vector assembled(x.Size());
  sum.AssembleDiagonal(assembled);

  // Recover the true diagonal from the action. Probing by global true DoF rather than local
  // index, so that the vector really is a unit vector across all ranks.
  mfem::Vector e(x.Size()), column(x.Size()), probed(x.Size());
  probed = 0.0;
  const HYPRE_BigInt offset = fespace.GetMyTDofOffset();
  for (HYPRE_BigInt g = 0; g < fespace.GlobalTrueVSize(); ++g)
  {
    const HYPRE_BigInt shifted = g - offset;
    const bool mine = shifted >= 0 && shifted < x.Size();
    e = 0.0;
    if (mine)
      e(shifted) = 1.0;
    sum.Mult(e, column);
    if (mine)
      probed(shifted) = column(shifted);
  }

  ASSERT_GT(probed.Normlinf(), 0.0);
  mfem::Vector difference(assembled);
  difference -= probed;
  EXPECT_LT(difference.Normlinf(), 1e-10 * probed.Normlinf());

  // The DIAG_ZERO nonlinear gradient and the DIAG_ONE linear operator must sum to exactly one
  // on essential rows. A diagonal of two is the bug FormJacobianMatrix fixes on the legacy path,
  // and a diagonal of zero makes OperatorJacobiSmoother abort.
  for (const auto tdof : ess_tdofs)
    EXPECT_DOUBLE_EQ(assembled(tdof), 1.0);
}

#endif
