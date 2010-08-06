#ifndef PDEBASE_H
#define PDEBASE_H

#include <string>

#include "Moose.h"
#include "MooseArray.h"
#include "MooseObject.h"
#include "PostprocessorInterface.h"
#include "FunctionInterface.h"

class PDEBase;
class MooseSystem;
class QuadraturePointData;

namespace libMesh
{
  class Mesh;
  class QGauss;
}

template<>
InputParameters validParams<PDEBase>();

/**
 * Base class for PDE objects (kernels, BCs, etc.)
 */
class PDEBase : public MooseObject, protected PostprocessorInterface, protected FunctionInterface
{
public:
  PDEBase(std::string name, MooseSystem &moose_system, InputParameters parameters, QuadraturePointData &data);
  virtual ~PDEBase();

  /**
   * Retrieve the name of the variable that this Kernel operates on
   */
  std::string varName() const;

  /**
   * The variable number that this kernel operates on.
   */
  unsigned int variable();

  /**
   * Whether or not the BC is integrated over the boundary.
   */
  bool isIntegrated();

  /**
   * Retrieve the names of the variables this Kernel is coupled to
   */
  const std::vector<std::string> & coupledTo() const;

  /**
   * This virtual gets called every time the subdomain changes.  This is useful for doing pre-calcualtions
   * that should only be done once per subdomain.  In particular this is where references to material
   * property vectors should be initialized.
   */
  virtual void subdomainSetup();

  /**
   * The time, after which this kernel will be active.
   */
  Real startTime();

  /**
   * The time, after which this kernel will be inactive.
   */
  Real stopTime();

  /**
   * Computes the volume integral for the current element.
   */
  virtual Real computeIntegral();

protected:
  // Integrable -------------------------------------------------------------------------

  /**
   * The mesh.
   */
  libMesh::Mesh & _mesh;

  /**
   * Current element
   */
  const Elem * & _current_elem;

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
   * The Finite Element type corresponding to the variable this
   * Object operates on.
   */
  FEType _fe_type;

  /**
   * If false the result of computeQpResidual() will overwrite the current Re entry instead of summing.
   * Right now it's only really used for computeSideResidual so Derichlet BC's can be computed exactly.
   */
  bool _integrated;

  /**
   * The current dimension of the mesh.
   */
  unsigned int & _dim;

  /**
   * Reference to quadrature point data
   */
  QuadraturePointData &_data;

  /**
   * Boundary quadrature rule.
   */
  QGauss * & _qrule;

  /**
   * XYZ coordinates of quadrature points
   */
  const std::vector<Point>& _q_point;

  /**
   * Jacobian pre-multiplied by the weight.
   */
  const std::vector<Real> & _JxW;

  /**
   * Shape function.
   */
  const std::vector<std::vector<Real> > & _phi;

  /**
   * Gradient of shape function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_phi;

  /**
   * Second derivative of shape function.
   */
  const std::vector<std::vector<RealTensor> > & _second_phi;

  /**
   * Current shape function.
   */
  unsigned int _i;

  /**
   * Current shape function while computing jacobians.
   * This should be used for the variable's shape functions, while _i
   * is used for the test function.
   */
  unsigned int _j;

  /**
   * Current _qrule quadrature point.
   */
  unsigned int _qp;

  /**
   * Just here for convenience.  Used in constructors... usually to deal with multiple dimensional stuff.
   */
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;

  /**
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual();

  /**
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * This is the virtual that derived classes should override for computing the volume integral of kernel.
   */
  virtual Real computeQpIntegral();


  // Transient --------------------------------------------------------------------------

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
   * Whether or not the current simulation is transient.
   */
  bool & _is_transient;

  /**
   * Whether or not the current simulation is Eigenvalue.
   */
  bool & _is_eigenvalue;

  /**
   * Current time step.
   */
  int & _t_step;

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  std::vector<Real> & _bdf2_wei;

  /**
   * Time discretization scheme: 0 - Implicit Euler, 1 - 2nd-order Backward Difference
   */
  short & _t_scheme;

  /**
   * The time, after which this kernel will be active.
   */
  Real _start_time;

  /**
   * The time, after which this kernel will be inactive.
   */
  Real _stop_time;

  // Coupling ---------------------------------------------------------------------------

  struct Variable {
    std::string _name;      /// the name of the variable
    unsigned int _num;      /// the number of the variable

    Variable() {}

    Variable(const std::string &name, unsigned int num) :
      _name(name),
      _num(num)
    {
    }
  };

  /**
   * List of all variables that are coupled to
   */
  std::map<std::string, std::vector<Variable> > _all_coupled_var;

  /**
   * List of all variables (names) this object is coupled to
   */
  std::vector<std::string> _coupled_to;

  /**
   * List of nonlinear variables coupled to
   */
  std::map<std::string, std::vector<Variable> > _coupled_vars;
  /**
   * List of auxiliary variables coupled to
   */
  std::map<std::string, std::vector<Variable> > _coupled_aux_vars;

  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual bool isCoupled(std::string varname, int i = 0);

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
  virtual bool isAux(std::string varname, int i = 0);

  /**
   * Returns the variable number of the coupled variable.
   */
  virtual unsigned int coupled(std::string varname, int i = 0);

  /**
   * Returns the number of coupled variables with name 'name'
   */
  virtual unsigned int coupledComponents(std::string varname);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual VariableValue & coupledValue(std::string varname, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual VariableGradient & coupledGradient(std::string varname, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled variable's second derivative.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual VariableSecond & coupledSecond(std::string varname, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at old time step.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual VariableValue & coupledValueOld(std::string varname, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value at older time step.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  virtual VariableValue & coupledValueOlder(std::string varname, int i = 0);

  /**
   * Returns a reference (that can be stored) to a coupled gradient of a variable's value at an old time step.
   *
   * @param name The name the kernel wants to refer to the variable as
   */
  virtual VariableGradient & coupledGradientOld(std::string varname, int i = 0);

};

#endif // PDEBASE_H
