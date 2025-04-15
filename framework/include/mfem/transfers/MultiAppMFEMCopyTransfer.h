#ifdef MFEM_ENABLED

#pragma once
#include <vector>

#include "MultiAppTransfer.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"
#include "MFEMProblem.h"

class MooseMesh;

//*
// Copy MFEMVariables between multiapps
// The variables must be of the same type and dimension
// and the MFEMMesh must be identical in both multiapps
// */

class MultiAppMFEMCopyTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppMFEMCopyTransfer(InputParameters const & params);
  void execute() override;
  auto const & getFromVarName(int i) { return _from_var_names.at(i); };
  auto const & getToVarName(int i) { return _to_var_names.at(i); };
  auto numFromVar() { return _from_var_names.size(); }
  auto numToVar() { return _to_var_names.size(); }

protected:
  std::vector<VariableName> _from_var_names;
  std::vector<AuxVariableName> _to_var_names;

  void transfer(MFEMProblem & to_problem, MFEMProblem & from_problem);

  void checkSiblingsTransferSupported() const override;
};

#endif
