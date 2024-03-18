//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AdvancedOutput.h"

class MooseMesh;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
class MeshFunction;
}

/**
 * Based class for providing re-positioning and oversampling support to output objects
 *
 * This class performs the actual oversampling calculations and makes the correct
 * changes to the libMesh::EquationsSystems() pointer (_es_ptr), i.e., this pointer is
 * will point to the oversampled system, if oversamping is utilized.
 *
 * The use of oversampling is triggered by setting the oversample input parameter to a
 * integer value greater than 0, indicating the number of refinements to perform.
 *
 * @see Exodus
 */
class OversampleOutput : public AdvancedOutput
{
public:
  static InputParameters validParams();

  OversampleOutput(const InputParameters & parameters);

  virtual ~OversampleOutput();

  virtual void initialSetup() override;
  virtual void meshChanged() override;
  virtual void outputStep(const ExecFlagType & type) override;

protected:
  /**
   * Performs the update of the solution vector for the oversample/re-positioned mesh
   */
  virtual void updateOversample();

  /// Appends the base class's file base string
  virtual void setFileBaseInternal(const std::string & file_base) override;

  /// The number of oversampling refinements
  const unsigned int _refinements;

  /// Flag indicating that oversampling is enabled
  bool _oversample;

  /// Flag for re-positioning
  bool _change_position;

private:
  /**
   * Setups the output object to produce re-positioned and/or oversampled results.
   * This is accomplished by creating a new, finer mesh that the existing solution is projected
   * upon. This function is called by the creating action (addOutputAction) and should not be called
   * by the user as it will create a memory leak if called multiple times.
   */
  void initOversample();

  /**
   * Clone mesh in preperation for re-positioning or oversampling.
   * This changes the pointer, _mesh_ptr, with a clone of the current mesh so that it may
   * be modified to perform the necessary oversample/positioning actions
   */
  void cloneMesh();

  /**
   * A vector of pointers to the mesh functions
   * This is only populated when the oversample() function is called, it must
   * be cleaned up by the destructor.
   */
  std::vector<std::vector<std::unique_ptr<MeshFunction>>> _mesh_functions;

  /// When oversampling, the output is shift by this amount
  Point _position;

  /// A flag indicating that the mesh has changed and the oversampled mesh needs to be re-initialized
  bool _oversample_mesh_changed;

  std::unique_ptr<EquationSystems> _oversample_es;
  std::unique_ptr<MooseMesh> _cloned_mesh_ptr;

  /// Oversample solution vector
  /* Each of the MeshFunctions keeps a reference to this vector, the vector is updated for the
   * current system
   * and variable before the MeshFunction is applied. This allows for the same MeshFunction object
   * to be
   * re-used, unless the mesh has changed due to adaptivity */
  std::unique_ptr<NumericVector<Number>> _serialized_solution;
};
