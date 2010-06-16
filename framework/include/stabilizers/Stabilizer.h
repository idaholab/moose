#ifndef STABILIZER_H
#define STABILIZER_H

#include "Moose.h"

// LibMesh includes
#include "parameters.h"
#include "point.h"
#include "vector_value.h"
#include "quadrature_gauss.h"

// System includes
#include <string>

//forward declarations
class Stabilizer;
class MooseSystem;
class ElementData;

template<>
InputParameters validParams<Stabilizer>();

/**
 * Stabilizers compute modified test function spaces to stabilize oscillating solutions.
 */
class Stabilizer
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the initial condition in the input file.
   * @param moose_system The reference to the MooseSystem that this object is contained within
   * @param parameters The parameters object holding data for the class to use.
   */
  Stabilizer(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual ~Stabilizer();

  /**
   * This pure virtual must be overriden by derived classes!
   *
   * This is where the stabilization scheme should compute the test function space.
   * This usually entails multipliying _phi by something and storing it in _test
   */
  virtual void computeTestFunctions() = 0;

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

protected:

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
   * The Finite Element type corresponding to the variable this
   * Kernel operates on.
   */
  FEType _fe_type;

  /**
   * Current element
   */
  const Elem * & _current_elem;

  /**
   * Interior shape function.
   */
  const std::vector<std::vector<Real> > & _phi;

  /**
   * Interior test function.
   *
   * These are non-const so they can be modified for stabilization.
   */
  std::vector<std::vector<Real> > & _test;

  /**
   * Gradient of interior test function.
   */
  const std::vector<std::vector<RealGradient> > & _grad_test;

  /**
   * Current quadrature rule.
   */
  QGauss * & _qrule;

  /**
   * Current shape function.
   */
  unsigned int _i;

  /**
   * Current _qrule quadrature point.
   */
  unsigned int _qp;

private:

  // Just here to satisfy the pure virtual
  virtual Real computeQpResidual();
};

#endif //STABILIZER_H
