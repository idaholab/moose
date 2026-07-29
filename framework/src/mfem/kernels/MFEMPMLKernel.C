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

namespace
{
/// Radial and tangential eigenvalues of the requested tensor, given the stretch factors.
///
/// J = J_r (rhat x rhat) + J_t (I - rhat x rhat) is already a spectral decomposition, and J is
/// complex symmetric, so J^T J = J^2. Both tensors are therefore diagonal in that same basis and
/// their eigenvalues follow one at a time, which avoids assembling J or inverting it per
/// integration point. The caller rebuilds the Cartesian tensor from these and rhat.
void
tensorEigenvalues(MFEMPMLMatrixCoefficient::Tensor tensor,
                  const std::complex<double> & radial_factor,
                  const std::complex<double> & tangential_factor,
                  int dim,
                  std::complex<double> & radial_eigenvalue,
                  std::complex<double> & tangential_eigenvalue)
{
  const std::complex<double> determinant =
      (dim == 2) ? radial_factor * tangential_factor
                 : radial_factor * tangential_factor * tangential_factor;

  if (tensor == MFEMPMLMatrixCoefficient::CURL)
  {
    radial_eigenvalue = radial_factor * radial_factor / determinant;
    tangential_eigenvalue = tangential_factor * tangential_factor / determinant;
  }
  else
  {
    radial_eigenvalue = determinant / (radial_factor * radial_factor);
    tangential_eigenvalue = determinant / (tangential_factor * tangential_factor);
  }
}
}

void
MFEMPMLMatrixCoefficient::Eval(mfem::DenseMatrix & K,
                               mfem::ElementTransformation & T,
                               const mfem::IntegrationPoint & ip)
{
  const int dim = _stretch.dim();
  double coordinates[3];
  mfem::Vector point(coordinates, 3);
  T.Transform(ip, point);

  mfem::Vector radial_direction;
  std::complex<double> radial_factor, tangential_factor;
  _stretch.evaluate(point, radial_direction, radial_factor, tangential_factor);

  std::complex<double> radial_eigenvalue, tangential_eigenvalue;
  tensorEigenvalues(
      _tensor, radial_factor, tangential_factor, dim, radial_eigenvalue, tangential_eigenvalue);

  const double base = _base_coefficient.Eval(T, ip);
  const double radial =
      base * ((_part == RE) ? radial_eigenvalue.real() : radial_eigenvalue.imag());
  const double tangential =
      base * ((_part == RE) ? tangential_eigenvalue.real() : tangential_eigenvalue.imag());

  // M = lambda_t I + (lambda_r - lambda_t) rhat (x) rhat
  K.SetSize(dim);
  for (const auto i : make_range(dim))
    for (const auto j : make_range(dim))
      K(i, j) = (radial - tangential) * radial_direction[i] * radial_direction[j] +
                ((i == j) ? tangential : 0.0);
}

double
MFEMPMLScalarCoefficient::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  double coordinates[3];
  mfem::Vector point(coordinates, 3);
  T.Transform(ip, point);

  mfem::Vector radial_direction;
  std::complex<double> radial_factor, tangential_factor;
  _stretch.evaluate(point, radial_direction, radial_factor, tangential_factor);

  const std::complex<double> inverse_determinant = 1.0 / (radial_factor * tangential_factor);
  const double base = _base_coefficient.Eval(T, ip);
  return base * ((_part == MFEMPMLMatrixCoefficient::RE) ? inverse_determinant.real()
                                                         : inverse_determinant.imag());
}

InputParameters
MFEMPMLKernel::validParams()
{
  InputParameters params = MFEMComplexKernel::validParams();
  params.addClassDescription(
      "Base class for radial perfectly matched layer complex bilinear form kernels.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of the base scalar coefficient to scale the integrator by.");
  params.addRequiredParam<Real>(
      "decay_coefficient",
      "Decay coefficient of the layer, equal to the tuning constant divided by the wavenumber.");
  params.addParam<Real>("decay_polynomial", 2.0, "Polynomial order of the stretch profile.");
  params.addParam<std::vector<Real>>(
      "reference_point",
      {},
      "Point that depth into the layer is measured from. Defaults to the barycenter of the mesh.");
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

std::unique_ptr<MFEMPMLStretch>
MFEMPMLKernel::makeStretch()
{
  mfem::ParMesh & mesh = getMFEMProblem().mesh().getMFEMParMesh();

  // An empty reference point leaves the choice to MFEMPMLStretch, which uses the mesh barycenter.
  const auto & supplied_point = getParam<std::vector<Real>>("reference_point");
  if (!supplied_point.empty() && (int)supplied_point.size() != mesh.Dimension())
    mooseError("MFEMPMLKernel: reference_point must have one entry per spatial dimension, so ",
               mesh.Dimension(),
               " entries for this mesh, but ",
               supplied_point.size(),
               " were given.");

  mfem::Vector reference_point(supplied_point.size());
  for (const auto d : index_range(supplied_point))
    reference_point[d] = supplied_point[d];

  return std::make_unique<MFEMPMLStretch>(mesh,
                                          getSubdomainAttributes(),
                                          reference_point,
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
