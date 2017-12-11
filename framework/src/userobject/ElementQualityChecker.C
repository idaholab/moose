/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElementQualityChecker.h"
#include "MooseError.h"

#include "libmesh/elem_quality.h"
#include "libmesh/enum_elem_quality.h"

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

template <>
InputParameters
validParams<ElementQualityChecker>()
{
  InputParameters params = validParams<ElementUserObject>();
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
  params.set<MultiMooseEnum>("execute_on") = "initial";

  return params;
}

ElementQualityChecker::ElementQualityChecker(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _m_type(getParam<MooseEnum>("metric_type").getEnum<ElemQuality>()),
    _has_upper_bound(isParamValid("upper_bound")),
    _has_lower_bound(isParamValid("lower_bound")),
    _upper_bound(_has_upper_bound ? getParam<Real>("upper_bound") : 0.0),
    _lower_bound(_has_lower_bound ? getParam<Real>("lower_bound") : 0.0),
    _failure_type(getParam<MooseEnum>("failure_type").getEnum<FailureType>())
{
  if (_has_lower_bound && _has_upper_bound && _lower_bound >= _upper_bound)
    mooseError("Provided lower bound should be less than provided upper bound!");
}

void
ElementQualityChecker::initialize()
{
  _m_values.clear();
  _elem_ids.clear();
  _bypassed = 0;
}

void
ElementQualityChecker::execute()
{
  // obtain the available quality metric for current ElemType
  std::vector<ElemQuality> metrics_avail = libMesh::Quality::valid(_current_elem->type());

  // check whether the provided quality metric is applicable to current ElemType
  if (!checkMetricApplicability(_m_type, metrics_avail))
  {
    _bypassed = 1;
    return;
  }

  std::pair<Real, Real> default_bounds = _current_elem->qual_bounds(_m_type);
  std::pair<Real, Real> actual_bounds;
  if (_has_lower_bound && _has_upper_bound)
    actual_bounds = std::make_pair(_lower_bound, _upper_bound);
  else if (_has_lower_bound)
  {
    if (_lower_bound < default_bounds.second)
      actual_bounds = std::make_pair(_lower_bound, default_bounds.second);
    else
      mooseError("Provided lower bound should less than the default upper bound: ",
                 default_bounds.second);
  }
  else if (_has_upper_bound)
  {
    if (_upper_bound > default_bounds.first)
      actual_bounds = std::make_pair(default_bounds.first, _upper_bound);
    else
      mooseError("Provided upper bound should larger than the default lower bound: ",
                 default_bounds.first);
  }
  else
    actual_bounds = default_bounds;

  // calculate and save quality metric value for current element
  Real mv = _current_elem->quality(_m_type);
  _m_values.insert(mv);

  // check element quality metric, save ids of elements whose quality metrics exceeds the preset
  // bounds
  if (mv < actual_bounds.first || mv > actual_bounds.second)
    _elem_ids.insert(_current_elem->id());
}

void
ElementQualityChecker::threadJoin(const UserObject & uo)
{
  const ElementQualityChecker & eqc = static_cast<const ElementQualityChecker &>(uo);
  _m_values.insert(eqc._m_values.begin(), eqc._m_values.end());
  _elem_ids.insert(eqc._elem_ids.begin(), eqc._elem_ids.end());
  _bypassed += eqc._bypassed;
}

void
ElementQualityChecker::finalize()
{
  _communicator.set_union(_m_values);
  _communicator.set_union(_elem_ids);
  _communicator.sum(_bypassed);

  if (_bypassed)
  {
    switch (_failure_type)
    {
      case FailureType::WARNING:
        mooseWarning("Provided quality metric doesn't apply to certain element type!");
        break;

      case FailureType::ERROR:
        mooseError("Provided quality metric doesn't apply to certain element type!");
        break;

      default:
        mooseError("Unknown failure type!");
    }
  }

  Real m_sum = 0;
  for (std::set<Real>::iterator it = _m_values.begin(); it != _m_values.end(); ++it)
    m_sum += *it;

  Moose::out << libMesh::Quality::name(_m_type) << " Metric values:" << std::endl;
  Moose::out << "              Minimum: " << *_m_values.begin() << std::endl;
  Moose::out << "              Maximum: " << *_m_values.rbegin() << std::endl;
  Moose::out << "              Average: " << m_sum / _m_values.size() << std::endl;

  if (!_elem_ids.empty())
  {
    switch (_failure_type)
    {
      case FailureType::WARNING:
      {
        mooseWarning("WARNING: Bad element quality in terms of ",
                     libMesh::Quality::name(_m_type),
                     " quality metric for ",
                     _elem_ids.size(),
                     " elements.");
        break;
      }

      case FailureType::ERROR:
      {
        mooseError("ERROR: Bad element quality in terms of ",
                   libMesh::Quality::name(_m_type),
                   " quality metric for ",
                   _elem_ids.size(),
                   " elements.");
        break;
      }

      default:
        mooseError("Unknown failure type!");
    }

    Moose::out << "List of failed element IDs: " << std::endl;
    for (std::set<unsigned int>::iterator it = _elem_ids.begin(); it != _elem_ids.end(); ++it)
      Moose::out << "  " << *it << std::endl;
  }
}

bool
ElementQualityChecker::checkMetricApplicability(ElemQuality elem_metric,
                                                std::vector<ElemQuality> elem_metrics)
{
  bool has_metric = false;

  for (unsigned int i = 0; i < elem_metrics.size(); ++i)
    if (elem_metric == elem_metrics[i])
      has_metric = true;

  return has_metric;
}
