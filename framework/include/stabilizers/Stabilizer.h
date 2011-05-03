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

#ifndef STABILIZER_H
#define STABILIZER_H

// System includes
#include <string>

#include "Moose.h"
#include "MooseObject.h"
#include "ParallelUniqueId.h"
#include "MaterialPropertyInterface.h"
// libMesh
#include "libmesh_common.h"
#include "elem.h"
#include "point.h"
#include "quadrature.h"
#include "vector_value.h"

class SubProblem;
class SystemBase;
class MooseVariable;
class Stabilizer;

template<>
InputParameters validParams<Stabilizer>();

/**
 * Base class for deriving new stabiler objects
 *
 * Stabilizers compute modified test function spaces to stabilize oscillating solutions.
 */
class Stabilizer :
  public MooseObject,
  protected MaterialPropertyInterface
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  Stabilizer(const std::string & name, InputParameters parameters);

  virtual ~Stabilizer();

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable() { return _var; }

  /**
   * This pure virtual must be overridden by derived classes!
   *
   * This is where the stabilization scheme should compute the test function space.
   * This usually entails multiplying _phi by something and storing it in _test
   */
  virtual void computeTestFunctions() = 0;

protected:
  SubProblem & _subproblem;
  THREAD_ID _tid;                                       ///< thread ID

  MooseVariable & _var;                                 ///< variable this stabilizer acts on

  const Elem * & _current_elem;                         ///< current element we work on

  unsigned int _qp;                                     ///< quadrature point index
  const std::vector< Point > & _q_point;                ///< quadrature points
  QBase * & _qrule;                                     ///< quadrature rule
  const std::vector<Real> & _JxW;                       ///< transformed Jacobian weights

   unsigned int _i, _j;                                 ///< i-th and j-th index for enumerating test and shape functions
   // shape functions
   const std::vector<std::vector<Real> > & _phi;
   const std::vector<std::vector<RealGradient> > & _grad_phi;
  /**
   * Interior test function.
   *
   * These are non-const so they can be modified for stabilization.
   */
  std::vector<std::vector<Real> > & _test;

  /**
   * Gradient of interior test function.
   */
  std::vector<std::vector<RealGradient> > & _grad_test;
};

#endif //STABILIZER_H
