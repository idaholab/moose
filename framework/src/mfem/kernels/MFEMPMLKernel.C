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

void
MFEMPMLDiagMatrixCoefficient::stretchFunction(const mfem::Vector & x,
                                              std::vector<std::complex<double>> & dxs) const
{
  constexpr std::complex<double> zi(0.0, 1.0);
  const double n = _decay_polynomial;
  for (int i = 0; i < _dim; ++i)
  {
    dxs[i] = 1.0;
    // High side (only if a PML exists there, i.e. length > 0).
    if (_length(i, 1) > 0.0 && x(i) >= _comp_domain_bdr(i, 1))
    {
      const double coeff = n * _decay_coefficient / std::pow(_length(i, 1), n);
      dxs[i] = 1.0 + zi * coeff * std::abs(std::pow(x(i) - _comp_domain_bdr(i, 1), n - 1.0));
    }
    // Low side.
    if (_length(i, 0) > 0.0 && x(i) <= _comp_domain_bdr(i, 0))
    {
      const double coeff = n * _decay_coefficient / std::pow(_length(i, 0), n);
      dxs[i] = 1.0 + zi * coeff * std::abs(std::pow(x(i) - _comp_domain_bdr(i, 0), n - 1.0));
    }
  }
}

void
MFEMPMLDiagMatrixCoefficient::Eval(mfem::Vector & K,
                                   mfem::ElementTransformation & T,
                                   const mfem::IntegrationPoint & ip)
{
  double x[3];
  mfem::Vector transip(x, 3);
  T.Transform(ip, transip);

  std::vector<std::complex<double>> dxs(_dim);
  stretchFunction(transip, dxs);

  std::complex<double> det(1.0, 0.0);
  for (int i = 0; i < _dim; ++i)
    det *= dxs[i];

  K.SetSize(vdim);
  if (_tensor == C2)
  {
    // detJ (J^T J)^{-1}: diagonal entry det / dxs[i]^2.
    for (int i = 0; i < _dim; ++i)
    {
      const std::complex<double> val = det / (dxs[i] * dxs[i]);
      K(i) = (_part == RE) ? val.real() : val.imag();
    }
  }
  else if (_dim == 2)
  {
    // detJ^{-1} J^T J in 2D reduces to the scalar 1/det.
    const std::complex<double> val = 1.0 / det;
    K(0) = (_part == RE) ? val.real() : val.imag();
  }
  else
  {
    // detJ^{-1} J^T J in 3D: diagonal entry dxs[i]^2 / det.
    for (int i = 0; i < _dim; ++i)
    {
      const std::complex<double> val = (dxs[i] * dxs[i]) / det;
      K(i) = (_part == RE) ? val.real() : val.imag();
    }
  }
}

InputParameters
MFEMPMLKernel::validParams()
{
  InputParameters params = MFEMComplexKernel::validParams();
  params.addClassDescription(
      "Base class for perfectly-matched-layer-stretched complex bilinear-form kernels.");
  params.addParam<MFEMScalarCoefficientName>(
      "coefficient", "1.", "Name of the base scalar coefficient to scale the integrator by.");
  params.addRequiredParam<Real>(
      "decay_coefficient",
      "PML decay coefficient, equal to the tuning constant divided by the wavenumber (c/k).");
  params.addParam<Real>(
      "decay_polynomial", 2.0, "Polynomial order of the PML stretch profile.");
  return params;
}

MFEMPMLKernel::MFEMPMLKernel(const InputParameters & parameters,
                             MFEMPMLDiagMatrixCoefficient::Tensor tensor)
  : MFEMComplexKernel(parameters),
    _base_coef(getScalarCoefficient("coefficient")),
    _decay_coefficient(getParam<Real>("decay_coefficient")),
    _decay_polynomial(getParam<Real>("decay_polynomial")),
    _dim(getMFEMProblem().mesh().getMFEMParMesh().Dimension()),
    _comp_domain_bdr(getCompDomainBoundary()),
    _length(getLength(_comp_domain_bdr)),
    _pml_re(_dim,
            _comp_domain_bdr,
            _length,
            _decay_coefficient,
            _decay_polynomial,
            tensor,
            MFEMPMLDiagMatrixCoefficient::RE),
    _pml_im(_dim,
            _comp_domain_bdr,
            _length,
            _decay_coefficient,
            _decay_polynomial,
            tensor,
            MFEMPMLDiagMatrixCoefficient::IM),
    _scaled_re(_base_coef, _pml_re),
    _scaled_im(_base_coef, _pml_im)
{
}

mfem::Array2D<double>
MFEMPMLKernel::getCompDomainBoundary()
{
  const mfem::ParMesh & mesh = getMFEMProblem().mesh().getMFEMParMesh();

  // The inner PML boundary is the bounding box of the interior (non-PML) region, i.e. all elements
  // NOT in this kernel's block. For a box this is exactly where the PML starts; on a face with no
  // PML (e.g. a source face) the interior reaches the domain boundary there, giving length 0 on
  // that side (see computeLength), which the stretch skips (no divide-by-zero).
  const mfem::Array<int> & pml_attrs = getSubdomainAttributes();
  mfem::Vector inner_min(_dim), inner_max(_dim);
  inner_min = std::numeric_limits<double>::max();
  inner_max = std::numeric_limits<double>::lowest();
  for (const auto e : make_range(mesh.GetNE()))
  {
    if (pml_attrs.Find(mesh.GetAttribute(e)) != -1)
      continue; // skip PML elements; accumulate interior only
    mfem::Array<int> verts;
    mesh.GetElementVertices(e, verts);
    for (const auto v : verts)
    {
      const double * c = mesh.GetVertex(v);
      for (const auto d : make_range(_dim))
      {
        inner_min(d) = std::min(inner_min(d), c[d]);
        inner_max(d) = std::max(inner_max(d), c[d]);
      }
    }
  }
  MPI_Allreduce(
      MPI_IN_PLACE, inner_min.GetData(), _dim, MFEM_MPI_REAL_T, MPI_MIN, getMFEMProblem().getComm());
  MPI_Allreduce(
      MPI_IN_PLACE, inner_max.GetData(), _dim, MFEM_MPI_REAL_T, MPI_MAX, getMFEMProblem().getComm());

  mfem::Array2D<double> comp_domain_bdr(_dim, 2);
  for (const auto d : make_range(_dim))
  {
    comp_domain_bdr(d, 0) = inner_min(d);
    comp_domain_bdr(d, 1) = inner_max(d);
  }
  return comp_domain_bdr;
}

mfem::Array2D<double>
MFEMPMLKernel::getLength(const mfem::Array2D<double> & comp_domain_bdr)
{
  mfem::ParMesh & mesh = getMFEMProblem().mesh().getMFEMParMesh();
  mfem::Vector pmin, pmax;
  mesh.GetBoundingBox(pmin, pmax);

  mfem::Array2D<double> length(_dim, 2);
  for (const auto d : make_range(_dim))
  {
    length(d, 0) = comp_domain_bdr(d, 0) - pmin(d);
    length(d, 1) = pmax(d) - comp_domain_bdr(d, 1);
  }
  return length;
}

mfem::BilinearFormIntegrator *
MFEMPMLKernel::getRealBFIntegrator()
{
  return makeIntegrator(_scaled_re);
}

mfem::BilinearFormIntegrator *
MFEMPMLKernel::getImagBFIntegrator()
{
  return makeIntegrator(_scaled_im);
}

#endif
