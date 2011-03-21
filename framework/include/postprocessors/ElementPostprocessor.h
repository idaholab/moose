#ifndef ELEMENTPOSTPROCESSOR_H
#define ELEMENTPOSTPROCESSOR_H

#include "Postprocessor.h"
#include "Coupleable.h"
#include "MooseVariable.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
// libMesh
#include "elem.h"

class MooseVariable;

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor :
  public Postprocessor,
  public Coupleable,
  public TransientInterface,
  public MaterialPropertyInterface
{
public:
  ElementPostprocessor(const std::string & name, InputParameters parameters);

  unsigned int blockID() { return _block_id; }

  virtual Real computeIntegral();

  unsigned int coupledComponents(const std::string & varname);
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableGradient  & coupledGradient(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient  & coupledGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient  & coupledGradientOlder(const std::string & var_name, unsigned int comp = 0);

protected:
  /**
   * The block ID this postprocessor works on
   */
  unsigned int _block_id;

  MooseVariable & _var;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  const Elem * & _current_elem;

  VariableValue & _u;                                   /// Holds the solution at current quadrature points
  VariableValue & _u_old;                               /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             /// Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                               /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u_old;                           /// Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                         /// Holds the t-2 solution gradient at the current quadrature point.

  virtual Real computeQpIntegral() = 0;
};
 
#endif
