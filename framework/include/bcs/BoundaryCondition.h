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

#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

#include "MooseObject.h"
#include "MooseVariable.h"
#include "ParallelUniqueId.h"
#include "MooseArray.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
// libMesh
#include "elem.h"
#include "vector_value.h"
#include "tensor_value.h"

class MooseVariable;
class MooseMesh;
class Problem;
class SubProblemInterface;
class SystemBase;
class BoundaryCondition;

template<>
InputParameters validParams<BoundaryCondition>();

/**
 * Base class for creating new types of boundary conditions
 *
 */
class BoundaryCondition :
  public MooseObject,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  public GeometricSearchInterface
{
public:
  BoundaryCondition(const std::string & name, InputParameters parameters);

  /**
   * Gets boundary ID this BC is active on
   * @return the boudanry ID
   */
  unsigned int boundaryID() { return _boundary_id; }

  /**
   * Gets the varaible this BC is actve on
   * @return the variable
   */
  MooseVariable & variable() { return _var; }

protected:
  Problem & _problem;
  SubProblemInterface & _subproblem;
  SystemBase & _sys;
  THREAD_ID _tid;                                       /// thread id
  MooseVariable & _var;                                 /// variable this BC works on
  MooseMesh & _mesh;                                    /// Mesh this BC is defined on
  int _dim;                                             /// dimension of the mesh

  unsigned int _boundary_id;                            /// boundary ID this BC is active on

  const Elem * & _current_elem;                         /// current element (valid only for integrated BCs)
  unsigned int & _current_side;                         /// current side of the current element (valid only for integrated BCs)

  const std::vector<Point> & _normals;                  /// normals at quadrature points (valid only for integrated BCs)

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;
};

#endif /* BOUNDARYCONDITION_H */
