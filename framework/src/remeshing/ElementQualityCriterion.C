//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementQualityCriterion.h"

#include "ElementQualityChecker.h"
#include "FEProblemBase.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"

#include <limits>

registerMooseObject("MooseApp", ElementQualityCriterion);

InputParameters
ElementQualityCriterion::validParams()
{
  InputParameters params = RemeshCriterion::validParams();

  params.addClassDescription("Remeshes when the worst element quality of the mesh falls below a "
                             "threshold.");

  params.addRequiredParam<MooseEnum>("quality_metric",
                                     ElementQualityChecker::QualityMetricType(),
                                     "The quality metric to measure.");
  params.addRequiredParam<Real>(
      "threshold",
      "The criterion fires when the smallest value of 'quality_metric' over the "
      "active elements is below this.");

  return params;
}

ElementQualityCriterion::ElementQualityCriterion(const InputParameters & parameters)
  : RemeshCriterion(parameters),
    _quality_metric(getParam<MooseEnum>("quality_metric").getEnum<libMesh::ElemQuality>()),
    _threshold(getParam<Real>("threshold"))
{
}

bool
ElementQualityCriterion::shouldRemesh()
{
  // Elem::quality() is only meaningful for active elements
  Real local_minimum = std::numeric_limits<Real>::max();
  for (const auto & elem : evaluationMesh().getMesh().active_local_element_ptr_range())
    local_minimum = std::min(local_minimum, elem->quality(_quality_metric));

  return minimumBelowThreshold(local_minimum, _threshold);
}
