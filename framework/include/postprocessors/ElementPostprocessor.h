#ifndef ELEMENTPOSTPROCESSOR_H_
#define ELEMENTPOSTPROCESSOR_H_

#include "Postprocessor.h"
#include "Variable.h"
#include "TransientInterface.h"
// libMesh
#include "elem.h"

namespace Moose {
  class Variable;
}

//Forward Declarations
class ElementPostprocessor;

template<>
InputParameters validParams<ElementPostprocessor>();

class ElementPostprocessor :
  public Postprocessor,
  public Moose::TransientInterface
{
public:
  ElementPostprocessor(const std::string & name, InputParameters parameters);

  unsigned int blockID() { return _block_id; }
  
protected:
  /**
   * The block ID this postprocessor works on
   */
  unsigned int _block_id;

  Moose::Variable & _var;

  unsigned int _qp;
  const std::vector< Point > & _q_point;

  const Elem * & _current_elem;

  VariableValue & _u;                                   /// Holds the solution at current quadrature points
  VariableValue & _u_old;                               /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             /// Holds the t-2 solution at the current quadrature point.

  VariableGrad & _grad_u;                               /// Holds the solution gradient at the current quadrature points
  VariableGrad & _grad_u_old;                           /// Holds the previous solution gradient at the current quadrature point.
  VariableGrad & _grad_u_older;                         /// Holds the t-2 solution gradient at the current quadrature point.

  /**
   * Override the pure virtual... this function should NOT be overridden by other ElementPostprocessors
   */
  virtual Real computeQpResidual() { return 0; };
};
 
#endif
