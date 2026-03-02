//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MultiAppTransfer.h"

/**
 * Virtual base class for MultiApp transfers to and/or from MFEMProblems.
 */
class MFEMMultiAppTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MFEMMultiAppTransfer(InputParameters const & params);
  /// Set active source and destination problems, and execute variable transfer
  void execute() override;
  /// Check number of source and target child apps match for sibling transfer
  void checkSiblingsTransferSupported() const override;

  /// Getter for source variable name
  auto const & getFromVarName(int i) { return _from_var_names.at(i); }
  /// Getter for destination variable name
  auto const & getToVarName(int i) { return _to_var_names.at(i); }
  /// Return the number of source variables
  auto const numFromVar() { return _from_var_names.size(); }
  /// Return for the number of destination variables
  auto const numToVar() { return _to_var_names.size(); }

protected:
  /// Transfer all variables from active source problem to active destination problem.
  virtual void transferVariables() = 0;

  /// Set current problem to fetch source variables from
  void setActiveFromProblem(FEProblemBase & from_problem) { _active_from_problem = &from_problem; }
  /// Set current problem to fetch destination variables from
  void setActiveToProblem(FEProblemBase & to_problem) { _active_to_problem = &to_problem; }
  /// Getter for current problem containing source variables
  virtual FEProblemBase & getActiveFromProblem() { return *_active_from_problem; }
  /// Getter for current problem containing destination variables
  virtual FEProblemBase & getActiveToProblem() { return *_active_to_problem; }

  /// Templated method to check source and destination problems are of the expected types
  template <typename TO_PROBLEM, typename FROM_PROBLEM>
  void checkValidTransferProblemTypes()
  {
    auto bad_problem = [this]()
    {
      mooseError(
          type(),
          " is not compatible with the provided source and/or destination Problem types of the "
          "provided variables.");
    };

    if (hasToMultiApp())
    {
      if (!dynamic_cast<FROM_PROBLEM *>(&getToMultiApp()->problemBase()))
        bad_problem();
      for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
        if (getToMultiApp()->hasLocalApp(i) &&
            !dynamic_cast<TO_PROBLEM *>(&getToMultiApp()->appProblemBase(i)))
          bad_problem();
    }
    if (hasFromMultiApp())
    {
      if (!dynamic_cast<TO_PROBLEM *>(&getFromMultiApp()->problemBase()))
        bad_problem();
      for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
        if (getFromMultiApp()->hasLocalApp(i) &&
            !dynamic_cast<FROM_PROBLEM *>(&getFromMultiApp()->appProblemBase(i)))
          bad_problem();
    }
  }

private:
  /// Vector of source variable names to be transferred
  std::vector<VariableName> _from_var_names;
  /// Vector of destination variable names to transfer to
  std::vector<AuxVariableName> _to_var_names;
  /// Pointer to active source problem variable is being transferred from
  FEProblemBase * _active_to_problem{nullptr};
  /// Pointer to active destination problem variable is being transferred to
  FEProblemBase * _active_from_problem{nullptr};
};

#endif
