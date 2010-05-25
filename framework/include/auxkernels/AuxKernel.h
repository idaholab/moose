#ifndef AUXKERNEL_H
#define AUXKERNEL_H

// This should be removed!!!
#include "Kernel.h"

//forward declarations
class AuxKernel;
class MooseSystem;
class ElementData;

template<>
InputParameters validParams<AuxKernel>();

/** 
 * AuxKernels compute values at nodes.
 */
class AuxKernel
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

  /**
   * This virtual gets called every time the subdomain changes.  This is useful for doing pre-calcualtions
   * that should only be done once per subdomain.  In particular this is where references to material
   * property vectors should be initialized.
   */
  virtual void subdomainSetup();

  /**
   * The variable number that this kernel operates on.
   */
  unsigned int variable();

  /**
   * Retrieve name of the Kernel
   */
  std::string name() const;

  /**
   * Retrieve the name of the variable that this Kernel operates on
   */
  std::string varName() const;

  /**
   * Retrieve the names of the variables this Kernel is coupled to
   */
  const std::vector<std::string> & coupledTo() const;

protected:
  virtual Real computeValue() = 0;

  virtual Real computeQpResidual();
  
  /**
   * This Kernel's name.
   */
  std::string _name;

  /**
   * Reference to the MooseSystem that this Kernel is assocaited to
   */
  MooseSystem & _moose_system;

  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;

  /**
   * The thread id this kernel is associated with.
   */
  THREAD_ID _tid;

  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

  /**
   * Name of the variable being solved for.
   */
  std::string _var_name;

  /**
   * Whether or not this kernel is operating on an auxiliary variable.
   */
  bool _is_aux;

  /**
   * System variable number for this variable.
   */
  unsigned int _var_num;

  /**
   * Variable numbers of the coupled variables.
   */
  std::vector<unsigned int> _coupled_var_nums;

  /**
   * Variable numbers of the coupled auxiliary variables.
   */
  std::vector<unsigned int> _aux_coupled_var_nums;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_to;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_as;

  /**
   * Map from _as_ to the actual variable number.
   */
  std::map<std::string, unsigned int> _coupled_as_to_var_num;

  /**
   * Map from _as_ to the actual variable number for auxiliary variables.
   */
  std::map<std::string, unsigned int> _aux_coupled_as_to_var_num;

  /**
   * The Finite Element type corresponding to the variable this
   * Kernel operates on.
   */
  FEType _fe_type;

  /**
   * Current material
   */
  Material * & _material;

  /**
   * Current quadrature rule.
   */
  QGauss * & _qrule;

  /**
   * Current time.
   */
  Real & _t;

  /**
   * Current dt.
   */
  Real & _dt;

  /**
   * Old dt.
   */
  Real & _dt_old;

  /**
   * Current time step.
   */
  int & _t_step;

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  std::vector<Real> & _bdf2_wei;

  bool _nodal;

  Real & _u_aux;  
  Real & _u_old_aux;
  Real & _u_older_aux;

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
  bool isAux(std::string name);

  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  bool isCoupled(std::string name);

  /**
   * Returns the variable number of the coupled variable.
   */
  unsigned int coupled(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<Real> & coupledVal(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<RealGradient> & coupledGrad(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's second derivative.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<RealTensor> & coupledSecond(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at old time step.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<Real> & coupledValOld(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at older time step.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  MooseArray<Real> & coupledValOlder(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled gradient of a variable's value at an old time step.
   *
   * @param name The name the kernel wants to refer to the variable as
   */
  MooseArray<RealGradient> & coupledGradValOld(std::string name);

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
};

#endif //AUXKERNEL_H
