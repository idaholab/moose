//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//*

#include "NLCurlCurlPAKernels.h"

namespace Moose::MFEM
{

// For now, no multithreading. this will be very slow
void
NLCurlCurlGradPASetup(const int Q1D,
                      const int NE,
                      const mfem::Array<mfem::real_t> & w,
                      const mfem::Vector & j,
                      const mfem::Vector & c,
                      mfem::Vector & k_coeff,
                      mfem::Vector & dk_coeff,
                      mfem::Vector & op,
                      mfem::real_t curlu_zero_tol)
{

  // number of quadpoints per element total
  const int NQ = Q1D * Q1D * Q1D;

  // next, turn all the important stuff into tensors
  auto W = w.Read();
  auto J = mfem::Reshape(j.Read(), NQ, 3, 3, NE);
  // hardcoding 1 here, since k expected to be scalar function
  auto K = mfem::Reshape(k_coeff.Read(), 1, NQ, NE);
  auto DK = mfem::Reshape(dk_coeff.Read(), 1, NQ, NE);

  // the Curl CoefficientVector
  auto C = mfem::Reshape(c.Read(), 3, NQ, NE);

  // finally, our operator
  auto Diag = mfem::Reshape(op.Write(), NQ, 6, NE);

  mfem::forall(NE * NQ,
               [=] MFEM_HOST_DEVICE(int idx)
               {
                 const int q = idx % NQ;
                 const int e = idx / NQ;
                 const mfem::real_t J11 = J(q, 0, 0, e);
                 const mfem::real_t J21 = J(q, 1, 0, e);
                 const mfem::real_t J31 = J(q, 2, 0, e);
                 const mfem::real_t J12 = J(q, 0, 1, e);
                 const mfem::real_t J22 = J(q, 1, 1, e);
                 const mfem::real_t J32 = J(q, 2, 1, e);
                 const mfem::real_t J13 = J(q, 0, 2, e);
                 const mfem::real_t J23 = J(q, 1, 2, e);
                 const mfem::real_t J33 = J(q, 2, 2, e);
                 const mfem::real_t detJ = J11 * (J22 * J33 - J32 * J23) -
                                           J21 * (J12 * J33 - J32 * J13) +
                                           J31 * (J12 * J23 - J22 * J13);
                 const mfem::real_t beta = W[q] / detJ;

                 // next, compute alpha. Simply read off the value of DK from this
                 // element/qpoint
                 const mfem::real_t alpha = DK(0, q, e);

                 // ditto for k
                 const mfem::real_t k = K(0, q, e);

                 // load the c_i - elements of the curl vector
                 mfem::real_t c1 = C(0, q, e);
                 mfem::real_t c2 = C(1, q, e);
                 mfem::real_t c3 = C(2, q, e);

                 // Unit curl vector
                 const mfem::real_t curl_mag = std::sqrt(c1 * c1 + c2 * c2 + c3 * c3);
                 const mfem::real_t inv_curl_mag = curl_mag > curlu_zero_tol ? 1.0 / curl_mag : 0.0;
                 c1 *= inv_curl_mag;
                 c2 *= inv_curl_mag;
                 c3 *= inv_curl_mag;

                 // next compute the g_i
                 const mfem::real_t g1 = J11 * c1 + J21 * c2 + J31 * c3;
                 const mfem::real_t g2 = J12 * c1 + J22 * c2 + J32 * c3;
                 const mfem::real_t g3 = J13 * c1 + J23 * c2 + J33 * c3;

                 // next the M_i. TODO - we can read these off the AddMultPA array
                 const mfem::real_t M11 = J11 * J11 + J21 * J21 + J31 * J31;
                 const mfem::real_t M22 = J12 * J12 + J22 * J22 + J32 * J32;
                 const mfem::real_t M33 = J13 * J13 + J23 * J23 + J33 * J33;
                 const mfem::real_t M12 = J11 * J12 + J21 * J22 + J31 * J32;
                 const mfem::real_t M13 = J11 * J13 + J21 * J23 + J31 * J33;
                 const mfem::real_t M23 = J12 * J13 + J22 * J23 + J32 * J33;

                 // Finally, let's write the 6 elements to the Diag tensor
                 // Do the upper triangle, in row major order
                 Diag(q, 0, e) = beta * (k * M11 + alpha * g1 * g1); // D11
                 Diag(q, 1, e) = beta * (k * M12 + alpha * g1 * g2); // D12
                 Diag(q, 2, e) = beta * (k * M13 + alpha * g1 * g3); // D13
                 Diag(q, 3, e) = beta * (k * M22 + alpha * g2 * g2); // D22
                 Diag(q, 4, e) = beta * (k * M23 + alpha * g2 * g3); // D23
                 Diag(q, 5, e) = beta * (k * M33 + alpha * g3 * g3); // D33
               });
}

void
NLCurlCurlPASetup(const int Q1D,
                  const int NE,
                  const mfem::Array<mfem::real_t> & w,
                  const mfem::Vector & j,
                  mfem::Vector & k_coeff,
                  mfem::Vector & op)
{
  // number of quadpoints per element total
  const int NQ = Q1D * Q1D * Q1D;

  // next, turn all the important stuff into tensors
  auto W = w.Read();
  auto J = mfem::Reshape(j.Read(), NQ, 3, 3, NE);
  // hardcoding 1 here, since k expected to be scalar function
  auto K = mfem::Reshape(k_coeff.Read(), 1, NQ, NE);

  // finally, our operator
  auto Diag = mfem::Reshape(op.Write(), NQ, 6, NE);

  // for each element
  for (int e = 0; e < NE; e++)
  {
    // for each qpoint
    for (int q = 0; q < NQ; q++)
    {
      const mfem::real_t J11 = J(q, 0, 0, e);
      const mfem::real_t J21 = J(q, 1, 0, e);
      const mfem::real_t J31 = J(q, 2, 0, e);
      const mfem::real_t J12 = J(q, 0, 1, e);
      const mfem::real_t J22 = J(q, 1, 1, e);
      const mfem::real_t J32 = J(q, 2, 1, e);
      const mfem::real_t J13 = J(q, 0, 2, e);
      const mfem::real_t J23 = J(q, 1, 2, e);
      const mfem::real_t J33 = J(q, 2, 2, e);
      const mfem::real_t detJ = J11 * (J22 * J33 - J32 * J23) - J21 * (J12 * J33 - J32 * J13) +
                                J31 * (J12 * J23 - J22 * J13);
      const mfem::real_t beta = W[q] / detJ;

      const mfem::real_t M11 = J11 * J11 + J21 * J21 + J31 * J31;
      const mfem::real_t M22 = J12 * J12 + J22 * J22 + J32 * J32;
      const mfem::real_t M33 = J13 * J13 + J23 * J23 + J33 * J33;
      const mfem::real_t M12 = J11 * J12 + J21 * J22 + J31 * J32;
      const mfem::real_t M13 = J11 * J13 + J21 * J23 + J31 * J33;
      const mfem::real_t M23 = J12 * J13 + J22 * J23 + J32 * J33;

      const mfem::real_t k = K(0, q, e);

      // finally, write into the diagonal operator
      Diag(q, 0, e) = beta * k * M11; // D11
      Diag(q, 1, e) = beta * k * M12; // D12
      Diag(q, 2, e) = beta * k * M13; // D13
      Diag(q, 3, e) = beta * k * M22; // D22
      Diag(q, 4, e) = beta * k * M23; // D23
      Diag(q, 5, e) = beta * k * M33; // D33
    }
  }
}
} // namespace Moose::MFEM
