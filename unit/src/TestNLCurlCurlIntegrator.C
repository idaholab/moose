#ifdef MOOSE_MFEM_ENABLED

#include "gtest/gtest.h"
#include "NLCurlCurlIntegrator.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

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

  const auto & ir = mfem::IntRules.Get(fespace.GetFE(0)->GetGeomType(), 2);

  Moose::MFEM::NLCurlCurlIntegrator integ(k_coeff, curlu_dk_dcurlu_coeff, curl_gf_coeff, &ir);

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

#endif
