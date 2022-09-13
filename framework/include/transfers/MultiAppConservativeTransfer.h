//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppFieldTransfer.h"

/**
 *  Transfers variables on possibly different meshes while conserving a user
 *  defined property (Postprocessor) for each variable
 */
class MultiAppConservativeTransfer : public MultiAppFieldTransfer
{
public:
  static InputParameters validParams();

  MultiAppConservativeTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * Add some extra work if necessary after execute(). For example, adjust the solution
   * to preserve some physics quality of interest.
   */
  virtual void postExecute();

protected:
  virtual std::vector<VariableName> getFromVarNames() const override { return _from_var_names; }
  virtual std::vector<AuxVariableName> getToVarNames() const override { return _to_var_names; }

  bool performAdjustment(const PostprocessorValue & from, const PostprocessorValue & to) const;

  /// Name of variables transfering from
  const std::vector<VariableName> _from_var_names;
  /// Name of variables transfering to
  const std::vector<AuxVariableName> _to_var_names;

  /// This values are used if a derived class only supports one variable
  VariableName _from_var_name;
  AuxVariableName _to_var_name;

  /// If this transfer is going to conserve the physics
  bool _preserve_transfer;
  /// Postprocessor evaluates an adjuster for the source physics
  std::vector<PostprocessorName> _from_postprocessors_to_be_preserved;
  /// Postprocessor evaluates an adjuster for the target physics
  std::vector<PostprocessorName> _to_postprocessors_to_be_preserved;

private:
  void adjustTransferedSolution(FEProblemBase * from_problem,
                                PostprocessorName & from_postprocessor,
                                FEProblemBase & to_problem,
                                PostprocessorName & to_postprocessor);

  void adjustTransferedSolutionNearestPoint(unsigned int i,
                                            FEProblemBase * from_problem,
                                            PostprocessorName & from_postprocessor,
                                            FEProblemBase & to_problem,
                                            PostprocessorName & to_postprocessor);

  bool _use_nearestpoint_pps;
  /// Whether the adjustment may be skipped when the postprocessor values are 0 / of different signs
  bool _allow_skipped_adjustment;
};
