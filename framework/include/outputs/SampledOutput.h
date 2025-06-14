//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * will point to the sampled system, if sampling/oversampling is utilized.
 *
 * The use of sampling is triggered by specifying one of the sampling parameters:
 * - refinements
 * - sampling block restriction
 * - a mesh file to sample on
 * - a position offset
 * @see Exodus
 */
class SampledOutput : public AdvancedOutput
{
public:
  static InputParameters validParams();

  SampledOutput(const InputParameters & parameters);

  virtual ~SampledOutput();

  virtual void initialSetup() override;
  virtual void meshChanged() override;
  virtual void outputStep(const ExecFlagType & type) override;

protected:
  /**
   * Performs the update of the solution vector for the sample/re-positioned mesh
   */
  virtual void updateSample();

  /// Appends the base class's file base string
  virtual void setFileBaseInternal(const std::string & file_base) override;

  /// The number of oversampling refinements
  const unsigned int _refinements;

  /// Flag indicating another file is being used for the sampling
  const bool _using_external_sampling_file;

  /// Flag for re-positioning
  const bool _change_position;

  /// Flag indicating that the sampled output should be used to re-sample the underlying EquationSystem of the output
  bool _use_sampled_output;

private:
  /**
   * Setups the output object to produce re-positioned and/or sampled results.
   * This is accomplished by creating a new, finer mesh that the existing solution is projected
   * upon. This function is called by the creating action (addOutputAction) and should not be called
   * by the user as it will create a memory leak if called multiple times.
   */
  void initSample();

  /**
   * Clone mesh in preperation for re-positioning or oversampling.
   * This changes the pointer, _mesh_ptr, with a clone of the current mesh so that it may
   * be modified to perform the necessary sample/positioning/block-restriction actions
   */
  void cloneMesh();

  /// Used to decide which variable is sampled at nodes, then output as a nodal variable for
  /// (over)sampling purposes
  /// If not sampled at nodes, it is sampled at centroids and output as a constant monomial
  bool isSampledAtNodes(const FEType & fe_type) const;

  /**
   * A vector of pointers to the mesh functions on the sampled mesh
   * This is only populated when the initSample() function is called, it must
   * be cleaned up by the destructor.
   * Outer-indexing by system
   * Inner-indexing for each variable in a system
   */
  std::vector<std::vector<std::unique_ptr<libMesh::MeshFunction>>> _mesh_functions;

  /// A vector of vectors that keeps track of the variable numbers in each system for each mesh function
  std::vector<std::vector<unsigned int>> _variable_numbers_in_system;

  /// When oversampling, the output is shift by this amount
  Point _position;

  /// A flag indicating that the mesh has changed and the sampled mesh needs to be re-initialized
  bool _sampling_mesh_changed;

  /// A flag tracking whether the sampling and source meshes match in terms of subdomains
  bool _mesh_subdomains_match;

  /// Flag indicating whether we are outputting in serial or parallel
  bool _serialize;

  /// Equation system holding the solution vectors for the sampled variables
  std::unique_ptr<EquationSystems> _sampling_es;

  /// Mesh used for sampling. The Output class' _mesh_ptr will refer to this mesh if sampling is being used
  std::unique_ptr<MooseMesh> _sampling_mesh_ptr;

  /// Sample solution vector
  /* Each of the MeshFunctions keeps a reference to this vector, the vector is updated for the
   * current system
   * and variable before the MeshFunction is applied. This allows for the same MeshFunction object
   * to be
   * re-used, unless the mesh has changed due to adaptivity */
  std::unique_ptr<NumericVector<Number>> _serialized_solution;
};
