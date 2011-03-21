#ifndef BOUNDARYCONDITION_H
#define BOUNDARYCONDITION_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"

// libMesh
#include "elem.h"
#include "vector_value.h"

class MooseVariable;
class SubProblem;

class BoundaryCondition :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface
{
public:
  BoundaryCondition(const std::string & name, InputParameters parameters);
  virtual ~BoundaryCondition();

  unsigned int boundaryID() { return _boundary_id; }

  MooseVariable & variable() { return _var; }

  unsigned int coupledComponents(const std::string & varname);

protected:
  SubProblem & _problem;
  SystemBase & _sys;
  THREAD_ID _tid;
  MooseVariable & _var;
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
