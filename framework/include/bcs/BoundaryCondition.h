#ifndef BOUNDARYCONDITION_H_
#define BOUNDARYCONDITION_H_

#include "Object.h"
#include "Integrable.h"
#include "FunctionInterface.h"

// libMesh
#include "elem.h"
#include "vector_value.h"

namespace Moose {
class Variable;
class SubProblem;
}

class BoundaryCondition :
  public Object,
  public FunctionInterface
{
public:
  BoundaryCondition(const std::string & name, InputParameters parameters);
  virtual ~BoundaryCondition();

  unsigned int boundaryId() { return _boundary_id; }

  Moose::Variable & variable() { return _var; }

protected:
  Moose::SubProblem & _problem;
  Moose::Variable & _var;

  unsigned int _boundary_id;

  const Elem * & _current_elem;
  unsigned int & _current_side;

  Real & _t;
};


template<>
InputParameters validParams<BoundaryCondition>();

#endif /* BOUNDARYCONDITION_H_ */
