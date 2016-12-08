/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef OVERSAMPLEOUTPUT_H
#define OVERSAMPLEOUTPUT_H

// MOOSE includes
#include "FileOutput.h"

// Forward declerations
class OversampleOutput;
class MooseMesh;

// libMesh forward declarations
namespace libMesh
{
template <typename T> class NumericVector;
class MeshFunction;
}


template<>
InputParameters validParams<OversampleOutput>();

/**
 * Based class for providing re-positioning and oversampling support to output objects
 *
 * This class performs the actual oversampling calculations and makes the correct
 * changes to the libMesh::EquationsSystems() pointer (_es_ptr), i.e., this pointer is
 * will point to the oversampled system, if oversamping is utilized.
 *
 * Additionally, the class adds a pointer the the mesh object (_mesh_ptr) that again
 * points to the correct mesh depending on the use of oversampling.
 *
 * The use of oversampling is triggered by setting the oversample input parameter to a
 * integer value greater than 0, indicating the number of refinements to perform.
 *
 * @see Exodus
 */
class OversampleOutput :
  public FileOutput
{
public:

  /**
   * Class constructor
   *
   * If oversampling is desired the constructor will perform the correct initialization
   * required for oversampling.
   * @see initOversample()
   */
  OversampleOutput(const InputParameters & parameters);

  /**
   * Class destructor
   *
   * Cleans up the various objects associated with the oversample EquationsSystem and Mesh
   * objects.
   */
  virtual ~OversampleOutput();

  /**
   * Executes when the mesh alterted and sets a flag used by oversampling
   */
  virtual void meshChanged() override;

protected:

  /**
   * Performs the update of the solution vector for the oversample/re-positioned mesh
   */
  virtual void updateOversample();

  /**
   * A convenience pointer to the current mesh (reference or displaced depending on "use_displaced")
   */
  MooseMesh * _mesh_ptr;

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
  /* Each of the MeshFunctions keeps a reference to this vector, the vector is updated for the current system
   * and variable before the MeshFunction is applied. This allows for the same MeshFunction object to be
   * re-used, unless the mesh has changed due to adaptivity */
  std::unique_ptr<NumericVector<Number> > _serialized_solution;
};

#endif // OVERSAMPLEOUTPUT_H
