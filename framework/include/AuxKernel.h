#include "Kernel.h"

#ifndef AUXKERNEL_H
#define AUXKERNEL_H

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
  AuxKernel(std::string name,
            Parameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as);

  virtual ~AuxKernel(){}

  /**
   * Must be called _after_ Kernel::init()
   */
  static void init();
  
  static void reinit(const NumericVector<Number>& soln, const Node & node);

  void computeAndStore();
  
protected:
  virtual Real computeValue() = 0;

  virtual Real computeQpResidual()
  {
    return 0;
  }

  Real & _u_nodal;  
  Real & _u_old_nodal;
  Real & _u_older_nodal;

  Real & coupledValAux(std::string name);
  Real & coupledValOldAux(std::string name);
  Real & coupledValOlderAux(std::string name);

  static const NumericVector<Number> * _nonlinear_old_soln;
  static const NumericVector<Number> * _nonlinear_older_soln;
  
  static NumericVector<Number> * _aux_soln;
  static const NumericVector<Number> * _aux_old_soln;
  static const NumericVector<Number> * _aux_older_soln;
  
  /**
   * Value of the variables at the nodes.
   */
  static std::map<unsigned int, Real > _var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  static std::map<unsigned int, Real > _var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  static std::map<unsigned int, Real > _var_vals_older_nodal;

  /**
   * Holds the current dof numbers for each variable
   */
  static std::map<unsigned int, unsigned int> _aux_var_dofs;

  /**
   * Value of the variables at the nodes.
   */
  static std::map<unsigned int, Real > _aux_var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  static std::map<unsigned int, Real > _aux_var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  static std::map<unsigned int, Real > _aux_var_vals_older_nodal;
  
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
