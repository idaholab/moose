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
#include "FEProblemBase.h"

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
  /// Allow sibling transfers
  void checkSiblingsTransferSupported() const override {}

  /// Getter for source variable name
  const VariableName & getFromVarName(int i) const { return _from_var_names.at(i); }
  /// Getter for destination variable name
  const VariableName & getToVarName(int i) const { return _to_var_names.at(i); }
  /// Return the number of source variables
  unsigned int numFromVar() const { return _from_var_names.size(); }
  /// Return for the number of destination variables
  unsigned int numToVar() const { return _to_var_names.size(); }

protected:
  /// Transfer all variables from active source problem to active destination problem.
  virtual void transferVariables(bool is_target_local) = 0;

  /// Set current problem to fetch source variables from
  void setActiveFromProblem(FEProblemBase & from_problem, const unsigned int global_app_index)
  {
    _active_from_problem = &from_problem;
    _active_from_global_app_index = global_app_index;
  }
  /// Set current problem to fetch destination variables from
  void setActiveToProblem(FEProblemBase & to_problem, const unsigned int global_app_index)
  {
    _active_to_problem = &to_problem;
    _active_to_global_app_index = global_app_index;
  }
  /// Getter for current problem containing source variables
  virtual FEProblemBase & getActiveFromProblem() { return *_active_from_problem; }
  /// Getter for current problem containing destination variables
  virtual FEProblemBase & getActiveToProblem() { return *_active_to_problem; }
  /// Getter for current destination problem global app index.
  virtual unsigned int & getActiveToProblemGlobalAppIndex() { return _active_to_global_app_index; }
  /// Getter for current destination problem global app index.
  virtual unsigned int & getActiveFromProblemGlobalAppIndex()
  {
    return _active_from_global_app_index;
  }

  /// Getter for the active source transform
  const MultiAppCoordTransform & getActiveFromTransform() const
  {
    return *_from_transforms[_active_from_global_app_index];
  }
  /// Getter for the active destination transform
  const MultiAppCoordTransform & getActiveToTransform() const
  {
    return *_to_transforms[_active_to_global_app_index];
  }
  /// Map a point in the active destination app frame to the active source app frame
  libMesh::Point mapPointToActiveSourceFrame(const Point & point_in_target_frame) const
  {
    return getActiveFromTransform().mapBack(getActiveToTransform()(point_in_target_frame));
  }
  /// Get libMesh EquationSystem, which may or may not be displaced
  libMesh::EquationSystems & getlibMeshEquationSystem(FEProblemBase & problem,
                                                      bool use_displaced) const;

  /// Set default value for transfers evaluated at points outside source mesh
  void setMFEMOutOfMeshValue(mfem::real_t mfem_out_of_mesh_value)
  {
    _mfem_out_of_mesh_value = mfem_out_of_mesh_value;
  }
  /// Getter for default value for transfers evaluated at points outside source mesh
  mfem::real_t getMFEMOutOfMeshValue() const { return _mfem_out_of_mesh_value; }

  /// Templated method to check source and destination problems are of the expected types
  template <Moose::FEBackend TO_BACKEND, Moose::FEBackend FROM_BACKEND>
  void checkValidTransferProblemTypes();

private:
  /// Vector of source variable names to be transferred
  const std::vector<VariableName> & _from_var_names;
  /// Vector of destination variable names to transfer to
  const std::vector<VariableName> & _to_var_names;
  /// Pointer to active destination problem variable is being transferred to
  FEProblemBase * _active_to_problem{nullptr};
  /// Pointer to active source problem variable is being transferred from
  FEProblemBase * _active_from_problem{nullptr};
  /// Global app index for the active destination problem
  unsigned int _active_to_global_app_index{0};
  /// Global app index for the active source problem
  unsigned int _active_from_global_app_index{0};
  /// Default value to return for transfers from points outside the source mesh
  mfem::real_t _mfem_out_of_mesh_value{std::numeric_limits<mfem::real_t>::infinity()};
};

template <Moose::FEBackend TO_BACKEND, Moose::FEBackend FROM_BACKEND>
inline void
MFEMMultiAppTransfer::checkValidTransferProblemTypes()
{
  // Check if source sub-apps exist, and if so, if they are of the expected type
  if (hasFromMultiApp())
  {
    for (const auto i : make_range(getFromMultiApp()->numGlobalApps()))
      if (getFromMultiApp()->hasLocalApp(i) &&
          getFromMultiApp()->appProblemBase(i).feBackend() != FROM_BACKEND)
        paramError("from_multi_app", type() + " is incompatible with the source app's backend.");
  }
  else if (getToMultiApp()->problemBase().feBackend() != FROM_BACKEND)
    mooseError(type() + " is incompatible with this (the source) app's backend.");

  // Check if destination sub-apps exist, and if so, if they are of the expected type
  if (hasToMultiApp())
  {
    for (const auto i : make_range(getToMultiApp()->numGlobalApps()))
      if (getToMultiApp()->hasLocalApp(i) &&
          getToMultiApp()->appProblemBase(i).feBackend() != TO_BACKEND)
        paramError("to_multi_app", type() + " is incompatible with the destination app's backend.");
  }
  else if (getFromMultiApp()->problemBase().feBackend() != TO_BACKEND)
    mooseError(type() + " is incompatible with this (the destination) app's backend.");
}

#endif
