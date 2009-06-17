#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "Kernel.h"

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
   * Size everything.
   */
  static void sizeEverything();

  /**
   * Must be called _after_ Kernel::init()
   */
  static void init();

  /**
   * Nodal reinit.
   */
  static void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Node & node);

  /**
   * Elemental reinit.
   */
  static void reinit(THREAD_ID tid, const NumericVector<Number>& soln, const Elem & elem);

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

  static const NumericVector<Number> * _nonlinear_old_soln;
  static const NumericVector<Number> * _nonlinear_older_soln;
  
  static NumericVector<Number> * _aux_soln;
  static const NumericVector<Number> * _aux_old_soln;
  static const NumericVector<Number> * _aux_older_soln;

  /**
   * Holds the current dof numbers for each variable
   */
  static std::vector<std::map<unsigned int, unsigned int> > _aux_var_dofs;


  /*************
   * Nodal Stuff
   *************/
  
  /**
   * Holds the variable numbers of the nodal aux vars.
   */
  static std::vector<unsigned int> _nodal_var_nums;
  
  /**
   * Value of the variables at the nodes.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_older_nodal;

  /**
   * Value of the variables at the nodes.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_nodal;

  /**
   * Value of the variables at the nodes.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_old_nodal;

  /**
   * Value of the variables at the nodes at t-2.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_older_nodal;


  /*****************
   * Elemental Stuff
   *****************/

  /**
   * Holds the variable numbers of the elemental aux vars.
   */
  static std::vector<unsigned int> _element_var_nums;

  /**
   * Value of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  static std::vector<std::map<unsigned int, Real > > _var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _var_grads_older_element;

  /**
   * Value of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_element;

  /**
   * Value of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_old_element;

  /**
   * Value of the variables at the elements at t-2.
   */
  static std::vector<std::map<unsigned int, Real > > _aux_var_vals_older_element;

  /**
   * Gradient of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _aux_var_grads_element;

  /**
   * Gradient of the variables at the elements.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _aux_var_grads_old_element;

  /**
   * Gradient of the variables at the elements at t-2.
   */
  static std::vector<std::map<unsigned int, RealGradient > > _aux_var_grads_older_element;


  static Real integrateValue(const std::vector<Real> & vals, const std::vector<Real> & JxW, const std::vector<Point> & q_point);
  static RealGradient integrateGradient(const std::vector<RealGradient> & grads, const std::vector<Real> & JxW, const std::vector<Point> & q_point);

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
