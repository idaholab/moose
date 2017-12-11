/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELEMENTQUALITYCHECKER_H
#define ELEMENTQUALITYCHECKER_H

#include "ElementUserObject.h"

#include "libmesh/enum_elem_quality.h"

class ElementQualityChecker;

template <>
InputParameters validParams<ElementQualityChecker>();

class ElementQualityChecker : public ElementUserObject
{
public:
  ElementQualityChecker(const InputParameters & parameters);

  static MooseEnum QualityMetricType();
  static MooseEnum FailureMessageType();

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;

protected:
  bool checkMetricApplicability(ElemQuality elem_metric, std::vector<ElemQuality> elem_metrics);

  ElemQuality _m_type;
  const bool _has_upper_bound;
  const bool _has_lower_bound;
  Real _upper_bound;
  Real _lower_bound;

  // set to save quality metric value for all elements
  std::set<Real> _m_values;
  // set to save ids for all failed elements
  std::set<unsigned int> _elem_ids;
  // whether the element quality check is bypassed or not
  unsigned int _bypassed;

private:
  enum class FailureType
  {
    WARNING,
    ERROR
  };
  // the way how check failure should respond: warning or error
  const FailureType _failure_type;
};

#endif // ELEMENTQUALITYCHECKER_H
