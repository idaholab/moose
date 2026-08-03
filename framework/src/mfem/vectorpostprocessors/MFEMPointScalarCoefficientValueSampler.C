//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPointScalarCoefficientValueSampler.h"

#include "MFEMProblem.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", MFEMPointScalarCoefficientValueSampler);

namespace
{
mfem::ParMesh &
mainMesh(const InputParameters & parameters)
{
  auto & problem =
      static_cast<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem"));
  return problem.mesh().getMFEMParMesh();
}
}

InputParameters
MFEMPointScalarCoefficientValueSampler::validParams()
{
  InputParameters params = MFEMSamplerBase::validParams();
  params.addClassDescription("Sample a real scalar MFEM coefficient at specific points.");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient",
                                                     "The scalar coefficient to sample.");
  params.addRequiredParam<std::vector<Point>>("points",
                                              "The points where the coefficient is evaluated.");
  return params;
}

MFEMPointScalarCoefficientValueSampler::MFEMPointScalarCoefficientValueSampler(
    const InputParameters & parameters)
  : MFEMSamplerBase(parameters, parameters.get<std::vector<Point>>("points"), mainMesh(parameters)),
    _coefficient(nullptr),
    _interp_vals(_query_points.size()),
    _declared_vals(declareVector(getParam<MFEMScalarCoefficientName>("coefficient")))
{
  _declared_vals.resize(_query_points.size());
}

void
MFEMPointScalarCoefficientValueSampler::initialSetup()
{
  _coefficient = &getScalarCoefficient("coefficient");
  if (dynamic_cast<mfem::QuadratureFunctionCoefficient *>(_coefficient))
    paramError("coefficient",
               "Quadrature-function-backed coefficients can only be evaluated at the quadrature "
               "points configured by their quadrature rule and cannot be sampled at arbitrary "
               "points.");

  MFEMSamplerBase::initialSetup();

  const auto & point_codes = _finder.GetCode();
  for (const auto i : index_range(_query_points))
    if (PointLocationCode(point_codes[i]) == PointLocationCode::BORDER)
      mooseWarning(typeAndName(),
                   " found point ",
                   _query_points[i],
                   " on an element boundary. An arbitrary coefficient may be discontinuous "
                   "there, so the element selected by GSLIB will supply the sampled value.");
}

void
MFEMPointScalarCoefficientValueSampler::execute()
{
  mfem::Array<unsigned int> received_elements;
  mfem::Array<unsigned int> received_codes;
  mfem::Vector received_reference_points;
  _finder.DistributePointInfoToOwningMPIRanks(
      received_elements, received_reference_points, received_codes);

  const auto mesh_dim = _mesh.Dimension();
  mfem::Vector evaluated_values(received_elements.Size());
  for (const auto i : make_range(received_elements.Size()))
  {
    mfem::IntegrationPoint integration_point;
    if (mesh_dim == 1)
      integration_point.Set1(received_reference_points[i]);
    else if (mesh_dim == 2)
      integration_point.Set2(received_reference_points[2 * i],
                             received_reference_points[2 * i + 1]);
    else
      integration_point.Set3(received_reference_points[3 * i],
                             received_reference_points[3 * i + 1],
                             received_reference_points[3 * i + 2]);

    auto & transformation = *_mesh.GetElementTransformation(received_elements[i]);
    transformation.SetIntPoint(&integration_point);
    evaluated_values[i] = _coefficient->Eval(transformation, integration_point);
  }

  // With one scalar value per point, byNODES and byVDIM orderings are identical.
  _finder.DistributeInterpolatedValues(evaluated_values, 1, mfem::Ordering::byVDIM, _interp_vals);
}

void
MFEMPointScalarCoefficientValueSampler::finalizeValues()
{
  const auto * const interp_vals = _interp_vals.HostRead();
  for (const auto i : index_range(_declared_vals))
    _declared_vals[i] = interp_vals[i];
}

#endif // MOOSE_MFEM_ENABLED
