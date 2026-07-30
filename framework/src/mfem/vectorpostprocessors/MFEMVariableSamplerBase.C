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

namespace
{
CreateMooseEnumClass(L2AverageType, NONE, ARITHMETIC, HARMONIC);

mfem::ParMesh &
variableMesh(const InputParameters & parameters)
{
  auto & problem =
      static_cast<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem"));
  return const_cast<mfem::ParMesh &>(
      problem.getMFEMVariableMesh(parameters.get<VariableName>("variable")));
}
}

InputParameters
MFEMVariableSamplerBase::validParams()
{
  InputParameters params = MFEMSamplerBase::validParams();
  MFEMExecutedObject::addRequiredDependencyParam<VariableName>(
      params, "variable", "The variable that this VectorPostprocessor samples");
  MooseEnum avg_type(getL2AverageTypeOptions(), "ARITHMETIC", false);
  params.addParam<MooseEnum>("side_interpolation_type",
                             avg_type,
                             "Average type used when sampling L2 functions at element boundaries.");
  return params;
}

MFEMVariableSamplerBase::MFEMVariableSamplerBase(const InputParameters & parameters,
                                                 const std::vector<Point> & points)
  : MFEMSamplerBase(parameters, points, variableMesh(parameters)),
    _var_name(getParam<VariableName>("variable"))
{
  _finder.SetL2AvgType(static_cast<mfem::FindPointsGSLIB::AvgType>(
      getParam<MooseEnum>("side_interpolation_type").getEnum<L2AverageType>()));
}

void
MFEMVariableSamplerBase::initialSetup()
{
  MFEMSamplerBase::initialSetup();

  const auto continuity_type = getFESpaceContinuityType();
  if (continuity_type == mfem::FiniteElementCollection::CONTINUOUS)
    return;

  const auto & point_codes = _finder.GetCode();
  for (const auto i : index_range(_query_points))
    if (PointLocationCode(point_codes[i]) == PointLocationCode::BORDER)
      switch (continuity_type)
      {
        case mfem::FiniteElementCollection::DISCONTINUOUS:
          mooseWarning(typeAndName(),
                       " found a point on an element boundary but the FE space is discontinuous at "
                       "element boundaries: ",
                       _query_points[i],
                       ".");
          break;
        case mfem::FiniteElementCollection::TANGENTIAL:
          mooseWarning(typeAndName(),
                       " found point ",
                       _query_points[i],
                       " on an element boundary. The H(curl) finite element space has a continuous "
                       "tangential trace there, but the normal component may differ between "
                       "elements. The element selected by GSLIB will supply the sampled value.");
          break;
        case mfem::FiniteElementCollection::NORMAL:
          mooseWarning(typeAndName(),
                       " found point ",
                       _query_points[i],
                       " on an element boundary. The H(div) finite element space has a continuous "
                       "normal trace there, but the tangential component may differ between "
                       "elements. The element selected by GSLIB will supply the sampled value.");
          break;
      }
}

#endif // MOOSE_MFEM_ENABLED
