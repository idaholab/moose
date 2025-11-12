//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"

/**
 * Adds objects to perform hydrodynamic coupling at injection and production points.
 */
class CoupledInjectionProductionPhysics : public PhysicsBase
{
public:
  static InputParameters validParams();

  CoupledInjectionProductionPhysics(const InputParameters & parameters);

protected:
  /// Adds a PorousFlowPointSourceFromPostprocessor
  void addPPSourceDiracKernel(const Point & point,
                              const NonlinearVariableName & var,
                              const PostprocessorName & pp_name);
  /// Adds a Receiver
  void addReceiverPostprocessor(const PostprocessorName & pp_name);
  /// Adds a PointValue
  void addPointValuePostprocessor(const VariableName & var,
                                  const Point & point,
                                  const PostprocessorName & pp_name);
  /// Adds a MultiAppPostprocessorTransfer
  void addPostprocessorTransfer(const PostprocessorName & pp_name, bool from_multi_app);

  /// Injection points
  const std::vector<Point> & _injection_points;
  /// Production points
  const std::vector<Point> & _production_points;

  /// Injection and production points
  std::vector<Point> _points;
  /// Label for each point
  std::vector<std::string> _labels;

private:
  void addDiracKernels() override;
  void addPostprocessors() override;
  void addTransfers() override;
};
