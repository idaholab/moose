//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "DependencyResolverInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "UserObjectInterface.h"
#include "Restartable.h"
#include "PostprocessorInterface.h"
#include "MeshChangedInterface.h"
#include "OutputInterface.h"

// Forward declarations
class MooseMesh;
class SubProblem;
class FEProblemBase;
class SystemBase;
class Assembly;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;
class Adaptivity;

class Marker : public MooseObject,
               public BlockRestrictable,
               public SetupInterface,
               public DependencyResolverInterface,
               public MooseVariableDependencyInterface,
               public UserObjectInterface,
               public Restartable,
               public PostprocessorInterface,
               public MeshChangedInterface,
               public OutputInterface
{
public:
  static InputParameters validParams();

  Marker(const InputParameters & parameters);
  virtual ~Marker() {}

  /// This mirrors the main refinement flag values in libMesh in Elem::RefinementState but adds "dont_mark"
  enum MarkerValue
  {
    DONT_MARK = -1,
    COARSEN,
    DO_NOTHING,
    REFINE
  };

  /**
   * Helper function for getting the valid refinement flag states a marker can use as a MooseEnum.
   * @return A MooseEnum that is filled with the valid states.  These are perfectly transferable to
   * libMesh Elem::RefinementStates.
   */
  static MooseEnum markerStates();

  virtual void computeMarker();

  bool isActive() const;

  /**
   * Is called before any element looping is started so any "global" computation can be done.
   */
  virtual void markerSetup();

  virtual const std::set<std::string> & getRequestedItems() override;

  virtual const std::set<std::string> & getSuppliedItems() override;

protected:
  virtual MarkerValue computeElementMarker() = 0;

  /**
   * Get an ErrorVector that will be filled up with values corresponding to the indicator passed in.
   *
   * Note that this returns a reference... and the return value should be stored as a reference!
   *
   * @param indicator The name of the indicator to get an ErrorVector for.
   */
  ErrorVector & getErrorVector(std::string indicator);

  /**
   * This is used to get the values of _other_ Markers.  This is useful for making combo-markers
   * that
   * take multiple markers and combine them to make one.
   *
   * @param name The name of the _other_ Marker that you want to have access to.
   * @return A _reference_ that will hold the value of the marker in it's 0 (zeroth) position.
   */
  const MooseArray<Real> & getMarkerValue(std::string name);

  SubProblem & _subproblem;
  FEProblemBase & _fe_problem;
  Adaptivity & _adaptivity;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;

  MooseVariable & _field_var;
  const Elem * const & _current_elem;

  MooseMesh & _mesh;

  /// Depend Markers
  std::set<std::string> _depend;
  std::set<std::string> _supplied;
};
