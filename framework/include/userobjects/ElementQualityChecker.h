//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

#include "libmesh/enum_elem_quality.h"

class ElementQualityChecker : public ElementUserObject
{
public:
  static InputParameters validParams();

  ElementQualityChecker(const InputParameters & parameters);

  static MooseEnum QualityMetricType();
  static MooseEnum FailureMessageType();

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;

protected:
  bool checkMetricApplicability(const libMesh::ElemQuality & elem_metric,
                                const std::vector<libMesh::ElemQuality> & elem_metrics);

  libMesh::ElemQuality _m_type;
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
