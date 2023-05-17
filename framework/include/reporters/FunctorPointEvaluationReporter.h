//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "FunctorInterface.h"
#include <functional>

class FunctorPointEvaluationReporter : public GeneralReporter, NonADFunctorInterface
{
public:
  static InputParameters validParams();
  FunctorPointEvaluationReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  ///{
  /// Containers to hold the reporter names
  std::vector<std::string> _real_reporter_names;
  std::vector<std::string> _real_vector_reporter_names;
  std::vector<std::string> _point_vector_reporter_names;
  ///}

  ///{
  /// Containers to hold the points to evaluate the functors at to compute the reporter values
  std::vector<Point> _real_reporter_points;
  std::vector<Point> _real_vector_reporter_points;
  std::vector<Point> _point_vector_reporter_points;
  ///}

  ///{
  /// Containers to hold the reporter values
  std::vector<std::reference_wrapper<Real>> _real_reporter_values;
  std::vector<std::reference_wrapper<std::vector<Real>>> _real_vector_reporter_values;
  std::vector<std::reference_wrapper<std::vector<Point>>> _point_vector_reporter_values;
  ///}

  ///{
  /// Containers to hold the functors to compute the reporter values
  std::vector<const Moose::Functor<Real> *> _real_reporter_functors;
  std::vector<std::vector<const Moose::Functor<Real> *>> _real_vector_reporter_functors;
  std::vector<std::vector<const Moose::Functor<Real> *>> _point_vector_reporter_functors;
  ///}

  /// Point locator used to check that the point is local, to avoid evaluating out of domain
  /// and to form the functor arguments
  std::unique_ptr<PointLocatorBase> _pl;
};
