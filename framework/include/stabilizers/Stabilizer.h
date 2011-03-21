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


//forward declarations
class Stabilizer;

template<>
InputParameters validParams<Stabilizer>();

class SubProblem;
class SystemBase;
class MooseVariable;

/**
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
   * @param moose_system The reference to the MooseSystem that this object is contained within
   * @param parameters The parameters object holding data for the class to use.
   */
  Stabilizer(const std::string & name, InputParameters parameters);

  virtual ~Stabilizer();

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable() { return _var; }

  virtual void setup() { }

  /**
   * This pure virtual must be overridden by derived classes!
   *
   * This is where the stabilization scheme should compute the test function space.
   * This usually entails multiplying _phi by something and storing it in _test
   */
  virtual void computeTestFunctions() = 0;

protected:
  SubProblem & _subproblem;
  THREAD_ID _tid;

  MooseVariable & _var;

  const Elem * & _current_elem;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

   unsigned int _i, _j;
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
