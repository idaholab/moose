#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"

//forward declarations
class AuxKernel;
class AuxiliarySystem;

template<>
InputParameters validParams<AuxKernel>();

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  protected GeometricSearchInterface
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AuxKernel(const std::string & name, InputParameters parameters);

  virtual ~AuxKernel() {}

  void compute(NumericVector<Number> & sln);

  MooseVariable & variable() { return _var; }

  bool isNodal();

  // Coupleable /////
  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);
  virtual VariableGradient  & coupledGradient(const std::string & var_name);

protected:
  virtual Real computeValue() = 0;

  SubProblem & _problem;
  SystemBase & _sys;
  AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;
  MooseVariable & _var;
  int _dim;

  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  const Elem * & _current_elem;
  const Node * & _current_node;

  Real & _current_volume;

  /**
   * true if the kernel nodal, false if it is elemental
   */
  bool _nodal;

  unsigned int _qp;

  // Single Instance Variables
  Real & _real_zero;
  Array<Real> & _zero;
  Array<RealGradient> & _grad_zero;
  Array<RealTensor> & _second_zero;
};

#endif //AUXKERNEL_H
