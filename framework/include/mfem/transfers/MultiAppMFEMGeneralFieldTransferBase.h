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

#include <vector>

#include "MultiAppTransfer.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"
#include "MFEMProblem.h"

class MooseMesh;

//*
// Transfer MFEMVariables between multiapps.
// The variables and mesh may be of different types in the source 
// and destination apps.
// */

class MultiAppMFEMGeneralFieldTransferBase : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMGeneralFieldTransferBase(InputParameters const & params);
  void execute() override;
  void checkSiblingsTransferSupported() const override;
  
  auto const & getFromVarName(int i) { return _from_var_names.at(i); };
  auto const & getToVarName(int i) { return _to_var_names.at(i); };
  auto numFromVar() { return _from_var_names.size(); }
  auto numToVar() { return _to_var_names.size(); }

protected:
  std::vector<VariableName> _from_var_names;
  std::vector<AuxVariableName> _to_var_names;
  FEProblemBase * _active_to_problem {nullptr};
  FEProblemBase * _active_from_problem {nullptr};

  void setActiveToProblem(FEProblemBase & to_problem) {_active_to_problem = &to_problem;};
  void setActiveFromProblem(FEProblemBase & from_problem) {_active_from_problem = &from_problem;};  
  virtual FEProblemBase & getActiveToProblem() {return *_active_to_problem;};
  virtual FEProblemBase & getActiveFromProblem() {return *_active_from_problem;};

  virtual void transferVariables() = 0;

  template <typename TO_PROBLEM, typename FROM_PROBLEM>  
  void checkValidTransferProblemTypes()
  {
    auto bad_problem = [this]()
    {
      mooseError(type(),
                " only works with MFEMProblem based applications. Check that all your inputs "
                "involved in this transfer are MFEMProblem based");
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
  };
  
};
#endif
