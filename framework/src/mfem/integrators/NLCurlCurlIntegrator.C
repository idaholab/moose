//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//*

#ifdef MOOSE_MFEM_ENABLED

#include "NLCurlCurlIntegrator.h"

#include "libmesh/ignore_warnings.h"
#include "mfem/fem/integ/bilininteg_hcurl_kernels.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{

// The params are:
// const int Q1D - the number of quadpoints in each dimension
// (we skip coeff_dim, as we expect the functions to be scalar functions)
// const int NE - the number of elements
// const Array<real_t> & w, the quadrature weights - get these from the integration rule
// const Vector &j - the jacobian matrices in the form of a flat vector. Its length is
//    9 * number of elements * number of quadpoints, i.e. one 3x3 matrix for each quadpoint
//    in each element
// const Vector& c - this is the curl, evaluated at each qpoint
// Vector & k_coeff  - reference to the vector of k(s) evaluated at each quadpoint. length is
//    one per quadpoint per element
// Vector & dk_coeff - ditto for k'(s) / s
// Vector & op - this is the diagonal operator
static void NLCurlCurlGradPASetup(const int Q1D,
                                  const int NE,
                                  const mfem::Array<mfem::real_t> & w,
                                  const mfem::Vector & j,
                                  const mfem::Vector & c,
                                  mfem::Vector & k_coeff,
                                  mfem::Vector & dk_coeff,
                                  mfem::Vector & op);

// Same, but creates the op used for AddMultPA
// const int Q1D - the number of quadpoints in each dimension
// (we skip coeff_dim, as we expect the functions to be scalar functions)
// const int NE - the number of elements
// const Array<real_t> & w, the quadrature weights - get these from the integration rule
// const Vector &j - the jacobian matrices in the form of a flat vector. Its length is
//    9 * number of elements * number of quadpoints, i.e. one 3x3 matrix for each quadpoint
//    in each element
// Vector & k_coeff  - reference to the vector of k(s) evaluated at each quadpoint. length is
//    one per quadpoint per element
// Vector & op - this is the diagonal operator
static void NLCurlCurlPASetup(const int Q1D,
                              const int NE,
                              const mfem::Array<mfem::real_t> & w,
                              const mfem::Vector & j,
                              mfem::Vector & k_coeff,
                              mfem::Vector & op);

NLCurlCurlJacMatrixCoefficient::NLCurlCurlJacMatrixCoefficient(mfem::Coefficient & k,
                                                               mfem::Coefficient & curlu_dk_dcurlu,
                                                               mfem::VectorCoefficient & curlu_vec,
                                                               mfem::real_t curlu_zero_tol)
  : mfem::MatrixCoefficient(curlu_vec.GetVDim()),
    _k_coef(k),
    _curlu_dk_dcurlu_coef(curlu_dk_dcurlu),
    _curlu_zero_tol(curlu_zero_tol),
    _curlu_hat_coef(curlu_vec, _curlu_zero_tol)
{
}

void
NLCurlCurlJacMatrixCoefficient::SetTime(mfem::real_t t)
{
  MatrixCoefficient::SetTime(t);
  _k_coef.SetTime(t);
  _curlu_dk_dcurlu_coef.SetTime(t);
  _curlu_hat_coef.SetTime(t);
}

void
NLCurlCurlJacMatrixCoefficient::Eval(mfem::DenseMatrix & K,
                                     mfem::ElementTransformation & T,
                                     const mfem::IntegrationPoint & ip)
{
  const int dim = GetHeight();
  mfem::Vector curlu_hat(dim);

  _curlu_hat_coef.Eval(curlu_hat, T, ip);
  const mfem::real_t k = _k_coef.Eval(T, ip);
  const mfem::real_t curlu_dk_dcurlu = _curlu_dk_dcurlu_coef.Eval(T, ip);

  K.Diag(k, dim);
  for (int i = 0; i < dim; ++i)
    for (int j = 0; j < dim; ++j)
      K(i, j) += curlu_dk_dcurlu * curlu_hat(i) * curlu_hat(j);
}

NLCurlCurlIntegrator::NLCurlCurlIntegrator(mfem::Coefficient & k,
                                           mfem::Coefficient & curlu_dk_dcurlu,
                                           mfem::Coefficient & dk_dcurlu,
                                           mfem::VectorCoefficient & curlu_vec,
                                           mfem::real_t curlu_zero_tol,
                                           const mfem::IntegrationRule * ir)
  : mfem::NonlinearFormIntegrator(ir), // so the PA path sees the rule
    _curlcurl_res_integ(k, ir),
    _curlcurl_jac_matrix_coef(k, curlu_dk_dcurlu, curlu_vec, curlu_zero_tol),
    _curlcurl_jac_integ(_curlcurl_jac_matrix_coef, ir),
    _k_coef(k),
    _dk_dcurlu_coef(dk_dcurlu),
    _curlu_vec(curlu_vec)
{
}

void
NLCurlCurlIntegrator::AssembleElementVector(const mfem::FiniteElement & el,
                                            mfem::ElementTransformation & Tr,
                                            const mfem::Vector & elfun,
                                            mfem::Vector & elvect)
{
  _curlcurl_res_integ.AssembleElementVector(el, Tr, elfun, elvect);
}

void
NLCurlCurlIntegrator::AssembleElementGrad(const mfem::FiniteElement & el,
                                          mfem::ElementTransformation & Tr,
                                          const mfem::Vector & elfun,
                                          mfem::DenseMatrix & elmat)
{
  _curlcurl_jac_integ.AssembleElementGrad(el, Tr, elfun, elmat);
}

// The first argument is the current state vector, which we actually don't need, since u appears
// only embedded in the function of k and as a curl.
void
NLCurlCurlIntegrator::AssembleGradPA(const mfem::Vector & /*x*/,
                                     const mfem::FiniteElementSpace & fes)
{
  const mfem::IntegrationRule * ir = PreAssemblySetup(fes);

  mfem::CoefficientVector k_coeff(*_qspace, mfem::CoefficientStorage::FULL);
  k_coeff.Project(_k_coef);
  mfem::CoefficientVector dk_coeff(*_qspace, mfem::CoefficientStorage::FULL);
  dk_coeff.Project(_dk_dcurlu_coef);
  mfem::CoefficientVector curl_coeff(*_qspace, mfem::CoefficientStorage::FULL);
  curl_coeff.Project(_curlu_vec);

  // This doesn't clear out what's in the array
  _pa_grad_data.SetSize(_ndata * _nq * _ne, mfem::Device::GetMemoryType());

  NLCurlCurlGradPASetup(
      _quad1D, _ne, ir->GetWeights(), _geom->J, curl_coeff, k_coeff, dk_coeff, _pa_grad_data);
}

// This mirrors AssembleGradPA exactly. We are performing redundant work here, and we
// will fix that in subsequent PRs.
void
NLCurlCurlIntegrator::AssemblePA(const mfem::FiniteElementSpace & fes)
{
  const mfem::IntegrationRule * ir = PreAssemblySetup(fes);

  mfem::CoefficientVector k_coeff(*_qspace, mfem::CoefficientStorage::FULL);
  k_coeff.Project(_k_coef);

  _pa_res_data.SetSize(_ndata * _nq * _ne, mfem::Device::GetMemoryType());
  NLCurlCurlPASetup(_quad1D, _ne, ir->GetWeights(), _geom->J, k_coeff, _pa_res_data);
}

void
NLCurlCurlIntegrator::AddMultGradPA(const mfem::Vector & x, mfem::Vector & y) const
{
  mfem::CurlCurlIntegrator::ApplyPAKernels::Run(_dim,
                                                _dofs1D,
                                                _quad1D,
                                                _dofs1D,
                                                _quad1D,
                                                _symmetric,
                                                _ne,
                                                _mapsO->B,
                                                _mapsC->B,
                                                _mapsO->Bt,
                                                _mapsC->Bt,
                                                _mapsC->G,
                                                _mapsC->Gt,
                                                _pa_grad_data,
                                                x,
                                                y,
                                                false);
}

void
NLCurlCurlIntegrator::AddMultPA(const mfem::Vector & x, mfem::Vector & y) const
{
  mfem::CurlCurlIntegrator::ApplyPAKernels::Run(_dim,
                                                _dofs1D,
                                                _quad1D,
                                                _dofs1D,
                                                _quad1D,
                                                _symmetric,
                                                _ne,
                                                _mapsO->B,
                                                _mapsC->B,
                                                _mapsO->Bt,
                                                _mapsC->Bt,
                                                _mapsC->G,
                                                _mapsC->Gt,
                                                _pa_res_data,
                                                x,
                                                y,
                                                false);
}

void
NLCurlCurlIntegrator::AssembleGradDiagonalPA(mfem::Vector & diag) const
{
  mfem::CurlCurlIntegrator::DiagonalPAKernels::Run(_dim,
                                                   _dofs1D,
                                                   _quad1D,
                                                   _dofs1D,
                                                   _quad1D,
                                                   _symmetric,
                                                   _ne,
                                                   _mapsO->B,
                                                   _mapsC->B,
                                                   _mapsO->G,
                                                   _mapsC->G,
                                                   _pa_grad_data,
                                                   diag);
}

// This gets called by both AssemblePA and AssembleGradPA, since they
// both need to store the transformation jacobians.
const mfem::IntegrationRule *
NLCurlCurlIntegrator::PreAssemblySetup(const mfem::FiniteElementSpace & fes)
{
  const mfem::FiniteElement * fel = fes.GetTypicalFE();
  mfem::Mesh * mesh = fes.GetMesh();

  // crucial check to see if it casts into VTFE
  const mfem::VectorTensorFiniteElement * el =
      dynamic_cast<const mfem::VectorTensorFiniteElement *>(fel);

  mooseAssert(el, "Only VectorTensorFiniteElement is supported!");
  mooseAssert(el->GetDerivType() == mfem::FiniteElement::CURL, "Unknown kernel type");

  // Match mfem::CurlCurlIntegrator::AssemblePA: honour a prescribed rule, otherwise fall back
  // to the mass-integrator rule the PA kernels were written against.
  const mfem::IntegrationRule * rule =
      IntRule ? IntRule
              : &mfem::MassIntegrator::GetRule(*el, *el, *mesh->GetTypicalElementTransformation());
  mooseAssert(el->GetDim() == 3, "Following methods are only implemented in 3D");

  _nq = rule->GetNPoints();
  _dim = mesh->Dimension();
  mooseAssert(_dim == 3, "Following methods are only implemented in 3D");

  _ne = fes.GetNE();
  _geom = mesh->GetGeometricFactors(*rule, mfem::GeometricFactors::JACOBIANS);
  _mapsC = &el->GetDofToQuad(*rule, mfem::DofToQuad::TENSOR);
  _mapsO = &el->GetDofToQuadOpen(*rule, mfem::DofToQuad::TENSOR);
  _dofs1D = _mapsC->ndof;
  _quad1D = _mapsC->nqpt;

  mooseAssert(_dofs1D == _mapsO->ndof + 1 && _quad1D == _mapsO->nqpt,
              "Must be one fewer open basis function per _dim than closed per dim.");

  if (!_qspace || _qspace_mesh != mesh || _qspace_mesh_sequence != mesh->GetSequence() ||
      _qspace_ir != rule)
  {
    _qspace = std::make_unique<mfem::QuadratureSpace>(*mesh, *rule);
    _qspace_mesh = mesh;
    _qspace_mesh_sequence = mesh->GetSequence();
    _qspace_ir = rule;
  }

  return rule;
}

// For now, no multithreading. this will be very slow
void
NLCurlCurlGradPASetup(const int Q1D,
                      const int NE,
                      const mfem::Array<mfem::real_t> & w,
                      const mfem::Vector & j,
                      const mfem::Vector & c,
                      mfem::Vector & k_coeff,
                      mfem::Vector & dk_coeff,
                      mfem::Vector & op)
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

      // next, compute alpha. Simply read off the value of DK from this
      // element/qpoint
      const mfem::real_t alpha = DK(0, q, e);

      // ditto for k
      const mfem::real_t k = K(0, q, e);

      // load the c_i - elements of the curl vector
      const mfem::real_t c1 = C(0, q, e);
      const mfem::real_t c2 = C(1, q, e);
      const mfem::real_t c3 = C(2, q, e);

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
    }
  }
}

// For now, no multithreading. this will be very slow
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
}

#endif
