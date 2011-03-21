#ifndef AUXKERNEL_H_
#define AUXKERNEL_H_

#include "Object.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

//forward declarations
class AuxKernel;

template<>
InputParameters validParams<AuxKernel>();

namespace Moose
{
class AuxiliarySystem;
}

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel :
  public Object,
  public Moose::Coupleable
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AuxKernel(const std::string & name, InputParameters parameters);

  virtual ~AuxKernel() {}

  void compute(NumericVector<Number> & sln);

  bool isNodal();

  // Coupleable /////
  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);

protected:
  virtual Real computeValue() = 0;

  Moose::SubProblem & _problem;
  Moose::AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;
  Moose::Variable & _var;

  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  const Elem * & _current_elem;

  Real & _current_volume;


  /**
   * true if the kernel nodal, false if it is elemental
   */
  bool _nodal;

  unsigned int _qp;

};

#endif //AUXKERNEL_H
