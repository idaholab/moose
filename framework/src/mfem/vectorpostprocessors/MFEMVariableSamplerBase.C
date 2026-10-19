//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVariableSamplerBase.h"

#include "MFEMProblem.h"
#include "MooseEnum.h"
#include "SubProblem.h"

namespace Moose::MFEM
{
std::vector<Point>
generateLinePoints(const Point & start_point, const Point & end_point, unsigned int num_points)
{
  // initialize and populate vector with linearly-spaced points along line
  std::vector<Point> points;
  points.reserve(num_points);
  for (const auto i_point : make_range(num_points))
  {
    // fractional distance along line [0, 1]
    Real t = static_cast<Real>(i_point) / static_cast<Real>(num_points - 1);
    points.push_back(t * end_point + (1 - t) * start_point);
  }

  return points;
}
}

namespace
{
CreateMooseEnumClass(L2AverageType, NONE, ARITHMETIC, HARMONIC);

mfem::ParMesh &
variableMesh(const InputParameters & parameters)
{
  auto & problem =
      cast_ref<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem"));
  auto & variables = parameters.get<std::vector<VariableName>>("variables");
  auto & first_variable = variables[0];
  auto & mesh = const_cast<mfem::ParMesh &>(problem.getMFEMVariableMesh(first_variable));

  // ensure that all variables are on the same mesh
  for (const auto & variable : variables)
  {
    if (&problem.getMFEMVariableMesh(variable) != &mesh)
    {
      mooseError("In MFEMVariableSamplerBase:"
                 " all variables must be on the same mesh. Found: ",
                 variable,
                 " which is not on the same mesh as: ",
                 first_variable,
                 ".");
    }
  }

  return mesh;
}
}

InputParameters
MFEMVariableSamplerBase::validParams()
{
  InputParameters params = MFEMSamplerBase::validParams();
  MFEMExecutedObject::addRequiredDependencyParam<std::vector<VariableName>>(
      params, "variables", "The variables that this VectorPostprocessor samples");
  MooseEnum avg_type(getL2AverageTypeOptions(), "ARITHMETIC");
  params.addParam<MooseEnum>("side_interpolation_type",
                             avg_type,
                             "Average type used when sampling L2 functions at element boundaries.");
  return params;
}

MFEMVariableSamplerBase::MFEMVariableSamplerBase(const InputParameters & parameters,
                                                 const std::vector<Point> & points)
  : MFEMSamplerBase(parameters, points, variableMesh(parameters)),
    _var_names(getParam<std::vector<VariableName>>("variables"))
{
  _finder.SetL2AvgType(static_cast<mfem::FindPointsGSLIB::AvgType>(
      getParam<MooseEnum>("side_interpolation_type").getEnum<L2AverageType>()));
}

void
MFEMVariableSamplerBase::initialSetup()
{
  MFEMSamplerBase::initialSetup();

  // Early return if all sampled variables are continuous
  bool all_continuous = true;
  for (const auto i_var : index_range(_var_names))
  {
    const auto continuity_type = getFESpaceContinuityType(i_var);
    if (continuity_type != mfem::FiniteElementCollection::CONTINUOUS)
    {
      all_continuous = false;
      break;
    }
  }

  if (all_continuous)
    return;

  const auto & point_codes = _finder.GetCode();
  for (const auto i : index_range(_query_points))
    if (PointLocationCode(point_codes[i]) == PointLocationCode::BORDER)
      for (const auto i_var : index_range(_var_names))
      {
        const auto continuity_type = getFESpaceContinuityType(i_var);
        switch (continuity_type)
        {
          case mfem::FiniteElementCollection::DISCONTINUOUS:
            mooseWarning(
                typeAndName(),
                " found a point on an element boundary but the FE space is discontinuous at "
                "element boundaries: ",
                _query_points[i],
                ". ",
                _var_names[i_var],
                " is discontinuous.");
            break;
          case mfem::FiniteElementCollection::TANGENTIAL:
            mooseWarning(
                typeAndName(),
                " found point ",
                _query_points[i],
                " on an element boundary. The H(curl) finite element space has a continuous "
                "tangential trace there, but the normal component may differ between "
                "elements. The element selected by GSLIB will supply the sampled value.",
                _var_names[i_var],
                " has tangential continuity.");
            break;
          case mfem::FiniteElementCollection::NORMAL:
            mooseWarning(
                typeAndName(),
                " found point ",
                _query_points[i],
                " on an element boundary. The H(div) finite element space has a continuous "
                "normal trace there, but the tangential component may differ between "
                "elements. The element selected by GSLIB will supply the sampled value.",
                _var_names[i_var],
                " has normal continuity.");
            break;
        }
      }
}

#endif // MOOSE_MFEM_ENABLED
