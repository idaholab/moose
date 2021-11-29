//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementQualityChecker.h"
#include "MooseError.h"
#include "Conversion.h"

#include "libmesh/elem_quality.h"
#include "libmesh/enum_elem_quality.h"
#include "libmesh/string_to_enum.h"

MooseEnum
ElementQualityChecker::QualityMetricType()
{
  return MooseEnum("ASPECT_RATIO SKEW SHEAR SHAPE MAX_ANGLE MIN_ANGLE CONDITION DISTORTION TAPER "
                   "WARP STRETCH DIAGONAL ASPECT_RATIO_BETA ASPECT_RATIO_GAMMA SIZE JACOBIAN");
}

MooseEnum
ElementQualityChecker::FailureMessageType()
{
  return MooseEnum("WARNING ERROR", "WARNING");
}

registerMooseObject("MooseApp", ElementQualityChecker);

InputParameters
ElementQualityChecker::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Class to check the quality of each element using different metrics "
                             "from libmesh.");

  params.addRequiredParam<MooseEnum>("metric_type",
                                     ElementQualityChecker::QualityMetricType(),
                                     "Type of quality metric to be checked");
  params.addParam<Real>("upper_bound", "the upper bound for provided metric type");
  params.addParam<Real>("lower_bound", "The lower bound for provided metric type");
  params.addParam<MooseEnum>("failure_type",
                             ElementQualityChecker::FailureMessageType(),
                             "The way how the failure of quality metric check should respond");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;

  return params;
}

ElementQualityChecker::ElementQualityChecker(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _m_type(getParam<MooseEnum>("metric_type").getEnum<ElemQuality>()),
    _has_upper_bound(isParamValid("upper_bound")),
    _has_lower_bound(isParamValid("lower_bound")),
    _upper_bound(_has_upper_bound ? getParam<Real>("upper_bound") : 0.0),
    _lower_bound(_has_lower_bound ? getParam<Real>("lower_bound") : 0.0),
    _m_min(0),
    _m_max(0),
    _m_sum(0),
    _failure_type(getParam<MooseEnum>("failure_type").getEnum<FailureType>())
{
}

void
ElementQualityChecker::initialize()
{
  _m_min = 0;
  _m_max = 0;
  _m_sum = 0;
  _checked_elem_num = 0;
  _elem_ids.clear();
  _bypassed = false;
  _bypassed_elem_type.clear();
}

void
ElementQualityChecker::execute()
{
  // obtain the available quality metric for current ElemType
  std::vector<ElemQuality> metrics_avail = libMesh::Quality::valid(_current_elem->type());

  // check whether the provided quality metric is applicable to current ElemType
  if (!checkMetricApplicability(_m_type, metrics_avail))
  {
    _bypassed = true;
    _bypassed_elem_type.insert(Utility::enum_to_string(_current_elem->type()));

    return;
  }

  std::pair<Real, Real> default_bounds = _current_elem->qual_bounds(_m_type);
  std::pair<Real, Real> actual_bounds;
  if (_has_lower_bound && _has_upper_bound)
  {
    if (_lower_bound >= _upper_bound)
      mooseError("Provided lower bound should be less than provided upper bound!");

    actual_bounds = std::make_pair(_lower_bound, _upper_bound);
  }
  else if (_has_lower_bound)
  {
    if (_lower_bound >= default_bounds.second)
      mooseError("Provided lower bound should less than the default upper bound: ",
                 default_bounds.second);

    actual_bounds = std::make_pair(_lower_bound, default_bounds.second);
  }
  else if (_has_upper_bound)
  {
    if (_upper_bound <= default_bounds.first)
      mooseError("Provided upper bound should larger than the default lower bound: ",
                 default_bounds.first);

    actual_bounds = std::make_pair(default_bounds.first, _upper_bound);
  }
  else
    actual_bounds = default_bounds;

  // calculate and save quality metric value for current element
  Real mv = _current_elem->quality(_m_type);

  _checked_elem_num += 1;
  _m_sum += mv;
  if (mv > _m_max)
    _m_max = mv;
  else if (mv < _m_min)
    _m_min = mv;

  // check element quality metric, save ids of elements whose quality metrics exceeds the preset
  // bounds
  if (mv < actual_bounds.first || mv > actual_bounds.second)
    _elem_ids.insert(_current_elem->id());
}

void
ElementQualityChecker::threadJoin(const UserObject & uo)
{
  const ElementQualityChecker & eqc = static_cast<const ElementQualityChecker &>(uo);
  _elem_ids.insert(eqc._elem_ids.begin(), eqc._elem_ids.end());
  _bypassed_elem_type.insert(eqc._bypassed_elem_type.begin(), eqc._bypassed_elem_type.end());
  _bypassed |= eqc._bypassed;
  _m_sum += eqc._m_sum;
  _checked_elem_num += eqc._checked_elem_num;

  if (_m_min > eqc._m_min)
    _m_min = eqc._m_min;
  if (_m_max < eqc._m_max)
    _m_max = eqc._m_max;
}

void
ElementQualityChecker::finalize()
{
  _communicator.min(_m_min);
  _communicator.max(_m_max);
  _communicator.sum(_m_sum);
  _communicator.sum(_checked_elem_num);
  _communicator.set_union(_elem_ids);
  _communicator.max(_bypassed);
  _communicator.set_union(_bypassed_elem_type);

  if (_bypassed)
    mooseWarning("Provided quality metric doesn't apply to following element type: " +
                 Moose::stringify(_bypassed_elem_type));

  _console << libMesh::Quality::name(_m_type) << " Metric values:"
           << "\n";
  _console << "              Minimum: " << _m_min << "\n";
  _console << "              Maximum: " << _m_max << "\n";
  _console << "              Average: " << _m_sum / _checked_elem_num << "\n";

  if (!_elem_ids.empty())
  {
    switch (_failure_type)
    {
      case FailureType::WARNING:
      {
        mooseWarning("List of failed element IDs: ", Moose::stringify(_elem_ids));
        break;
      }

      case FailureType::ERROR:
      {
        mooseError("List of failed element IDs: ", Moose::stringify(_elem_ids));
        break;
      }

      default:
        mooseError("Unknown failure type!");
    }
  }

  _console << std::flush;
}

bool
ElementQualityChecker::checkMetricApplicability(const ElemQuality & elem_metric,
                                                const std::vector<ElemQuality> & elem_metrics)
{
  bool has_metric = false;

  for (unsigned int i = 0; i < elem_metrics.size(); ++i)
    if (elem_metric == elem_metrics[i])
      has_metric = true;

  return has_metric;
}
