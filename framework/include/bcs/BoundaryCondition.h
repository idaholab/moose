#ifndef BOUNDARYCONDITION_H_
#define BOUNDARYCONDITION_H_

#include "Object.h"
#include "Integrable.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"

// libMesh
#include "elem.h"
#include "vector_value.h"

namespace Moose {
class Variable;
class SubProblem;
}

class BoundaryCondition :
  public Object,
  public Moose::Coupleable,
  public FunctionInterface,
  public Moose::TransientInterface,
  public Moose::MaterialPropertyInterface,
  public Moose::PostprocessorInterface
{
public:
  BoundaryCondition(const std::string & name, InputParameters parameters);
  virtual ~BoundaryCondition();

  unsigned int boundaryID() { return _boundary_id; }

  Moose::Variable & variable() { return _var; }

protected:
  Moose::SubProblem & _problem;
  Moose::System & _sys;
  THREAD_ID _tid;
  Moose::Variable & _var;
  int _dim;

  unsigned int _boundary_id;

  const Elem * & _current_elem;
  unsigned int & _current_side;

  const std::vector<Point> & _normals;

  // Single Instance Variables
  Real & _real_zero;
  Array<Real> & _zero;
  Array<RealGradient> & _grad_zero;
  Array<RealTensor> & _second_zero;
};


template<>
InputParameters validParams<BoundaryCondition>();

#endif /* BOUNDARYCONDITION_H_ */
