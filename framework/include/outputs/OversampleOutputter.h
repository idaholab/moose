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

#ifndef OVERSAMPLEOUTPUTTER_H
#define OVERSAMPLEOUTPUTTER_H

// MOOSE includes
#include "OutputBase.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/equation_systems.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh_function.h"

// Forward declerations
class OversampleOutputter;

template<>
InputParameters validParams<OversampleOutputter>();

/**
 * Based class for providing oversampling support to output objects
 *
 * This class performs the actual oversampling calculations and makes the correct
 * changes to the libMesh::EquationsSystems() pointer (_es_ptr), i.e., this pointer is
 * will point to the oversampled system, if oversamping is utilized.
 *
 * Additionally, the class adds a pointer the the mesh object (_mesh_ptr) that again
 * points to the correct mesh depending on the use of oversampling.
 *
 * The use of oversampling is triggered by setting the oversample input parameter to a
 * interger value greater than 0, indicating the number of refinements to perform.
 *
 * @see Exodus
 */
class OversampleOutputter :
  public OutputBase
{
public:

  /**
   * Class constructor
   *
   * If oversampling is desired the constuctor will perform the correct initialization
   * required for oversampling.
   * @see initOversample()
   */
  OversampleOutputter(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   *
   * Cleans up the various objects associated with the oversample EquationsSystem and Mesh
   * objects.
   */
  virtual ~OversampleOutputter();

  /**
   * Overloaded output() that include oversampling
   */
  virtual void output();

protected:

  /**
   * Performs the update of the solution vector for the oversample mesh
   */
  virtual void oversample();

private:

  /**
   * Setups the output object to produce oversampled results.
   * This is accomplished by creating a new, finer mesh that the existing solution is projected
   * upon. This function is called by the creating action (addOutputAction) and should not be called
   * by the user as it will create a memory leak if called multiple times.
   */
  virtual void initOversample();

  /**
   * A pointer to the current mesh
   * When using oversampling this points to the created oversampled, which must
   * be cleaned up by the destructor.
   */
  MooseMesh * _mesh_ptr;

  /**
   * A vector of pointers to the mesh functions
   * This is only populated when the oversample() function is called, it must
   * be cleaned up by the destructor.
   */
  std::vector<std::vector<MeshFunction *> > _mesh_functions;

  /**
   * A pointer to the oversampled solution
   * When using oversampling this points to the created oversampled solution, which must
   * be cleaned up by the destructor.
   */
  NumericVector<Number> * _serialized_solution;

  /// The number of oversampling refinements (0 = do not oversample)
  const unsigned int _oversample;

  /// When oversampling, the output is shift by this amount
  Point _position;

};

#endif // OVERSAMPLEBSE_H
