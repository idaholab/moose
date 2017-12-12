/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
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
  bool checkMetricApplicability(const ElemQuality & elem_metric,
                                const std::vector<ElemQuality> & elem_metrics);

  ElemQuality _m_type;
  const bool _has_upper_bound;
  const bool _has_lower_bound;
  Real _upper_bound;
  Real _lower_bound;

  // minimum, maximum and summation of quality metric values of all checked elements
  Real _m_min;
  Real _m_max;
  Real _m_sum;
  // number of checked elements
  unsigned int _checked_elem_num;
  // set to save ids for all failed elements
  std::set<dof_id_type> _elem_ids;
  // whether the element quality check is bypassed or not
  bool _bypassed;
  // set to save bypassed element type
  std::set<std::string> _bypassed_elem_type;

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
