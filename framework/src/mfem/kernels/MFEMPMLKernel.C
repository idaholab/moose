//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPMLKernel.h"
#include "MFEMProblem.h"

#include <Eigen/Dense>

namespace
{
using Complex = std::complex<Real>;
using ComplexMatrix = Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor, 3, 3>;

/// The stretch Jacobian J = I + i grad(W), which is complex only through the explicit i.
ComplexMatrix
stretchJacobian(const mfem::DenseMatrix & stretch_gradient, int dim)
{
  ComplexMatrix jacobian = ComplexMatrix::Identity(dim, dim);

  for (const auto a : make_range(dim))
    for (const auto b : make_range(dim))
      jacobian(a, b) += Complex(0.0, stretch_gradient(a, b));

  return jacobian;
}
}

void
MFEMPMLMatrixCoefficient::Eval(mfem::DenseMatrix & K,
                               mfem::ElementTransformation & T,
                               const mfem::IntegrationPoint & ip)
{
  const int dim = _stretch.dim();
  T.SetIntPoint(&ip);

  mfem::DenseMatrix stretch_gradient;
  _stretch.stretchGradient(T, stretch_gradient);

  const ComplexMatrix jacobian = stretchJacobian(stretch_gradient, dim);
  const ComplexMatrix product = jacobian.transpose() * jacobian;
  const Complex determinant = jacobian.determinant();
  const ComplexMatrix tensor = (_tensor == CURL) ? ComplexMatrix(product / determinant)
                                                 : ComplexMatrix(product.inverse() * determinant);

  const double base = _base_coefficient.Eval(T, ip);

  K.SetSize(dim);
  for (const auto a : make_range(dim))
    for (const auto b : make_range(dim))
      K(a, b) = base * ((_part == RE) ? tensor(a, b).real() : tensor(a, b).imag());
}

double
MFEMPMLScalarCoefficient::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  T.SetIntPoint(&ip);

  mfem::DenseMatrix stretch_gradient;
  _stretch.stretchGradient(T, stretch_gradient);

  const Complex value = _base_coefficient.Eval(T, ip) /
                        stretchJacobian(stretch_gradient, _stretch.dim()).determinant();
  return (_part == MFEMPMLMatrixCoefficient::RE) ? value.real() : value.imag();
}

InputParameters
MFEMPMLKernel::validParams()
{
  InputParameters params = MFEMComplexKernel::validParams();
  params.addClassDescription(
      "Base class for perfectly matched layer complex bilinear form kernels.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of the base scalar coefficient to scale the integrator by.");
  params.addRequiredParam<Real>(
      "decay_coefficient",
      "Decay coefficient of the layer, equal to the tuning constant divided by the wavenumber.");
  params.addParam<Real>("decay_polynomial", 2.0, "Polynomial order of the stretch profile.");
  return params;
}

MFEMPMLKernel::MFEMPMLKernel(const InputParameters & parameters,
                             MFEMPMLMatrixCoefficient::Tensor tensor)
  : MFEMComplexKernel(parameters),
    _base_coefficient(getScalarCoefficient("coefficient")),
    _stretch(makeStretch()),
    _matrix_re(*_stretch, _base_coefficient, tensor, MFEMPMLMatrixCoefficient::RE),
    _matrix_im(*_stretch, _base_coefficient, tensor, MFEMPMLMatrixCoefficient::IM),
    _scalar_re(*_stretch, _base_coefficient, MFEMPMLMatrixCoefficient::RE),
    _scalar_im(*_stretch, _base_coefficient, MFEMPMLMatrixCoefficient::IM)
{
}

std::unique_ptr<MFEMPMLStretchVector>
MFEMPMLKernel::makeStretch()
{
  return std::make_unique<MFEMPMLStretchVector>(getMFEMProblem().mesh().getMFEMParMesh(),
                                                getSubdomainAttributes(),
                                                getParam<Real>("decay_coefficient"),
                                                getParam<Real>("decay_polynomial"),
                                                getMFEMProblem().getComm());
}

mfem::BilinearFormIntegrator *
MFEMPMLKernel::getRealBFIntegrator()
{
  return makeIntegrator(MFEMPMLMatrixCoefficient::RE);
}

mfem::BilinearFormIntegrator *
MFEMPMLKernel::getImagBFIntegrator()
{
  return makeIntegrator(MFEMPMLMatrixCoefficient::IM);
}

#endif
