#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "Kernel.h"

//forward declarations
class AuxKernel;

template<>
InputParameters validParams<AuxKernel>();

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel : public Kernel
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  AuxKernel(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~AuxKernel(){}

  void computeAndStore(THREAD_ID tid);

  bool isNodal();
  
  
protected:
  virtual Real computeValue() = 0;

  virtual Real computeQpResidual();
  
  bool _nodal;

  Real & _u_aux;  
  Real & _u_old_aux;
  Real & _u_older_aux;

  Real & coupledValAux(std::string name);
  Real & coupledValOldAux(std::string name);
  Real & coupledValOlderAux(std::string name);

  RealGradient & coupledGradAux(std::string name);
  RealGradient & coupledGradOldAux(std::string name);
  RealGradient & coupledGradOlderAux(std::string name);


  /*************
   * Nodal Stuff
   *************/
  /**
   * Current Node
   */
  const Node * & _current_node;


private:

  /**
   * Stuff we don't want AuxKernel classes to get access to from Kernel
   */
  Kernel::_u;
  Kernel::_u_old;
  Kernel::_u_older;
  Kernel::_grad_u;
  Kernel::_grad_u_old;
  Kernel::_grad_u_older;
  Kernel::_JxW;
  Kernel::_phi;
  Kernel::_dphi;
  Kernel::_q_point;
};

#endif //AUXKERNEL_H
