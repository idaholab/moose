#ifndef SIDEPOSTPROCESSOR_H_
#define SIDEPOSTPROCESSOR_H_

#include "Postprocessor.h"
#include "Variable.h"

//Forward Declarations
class SidePostprocessor;

template<>
InputParameters validParams<SidePostprocessor>();

class SidePostprocessor :
  public Postprocessor
{
public:
  SidePostprocessor(const std::string & name, InputParameters parameters);

  unsigned int boundaryID() { return _boundary_id; }


protected:
  Moose::Variable & _var;

  unsigned int _boundary_id;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  const std::vector<Point> & _normals;

  // unknown
  const VariableValue & _u;
  const VariableGrad & _grad_u;

  /**
   * Override the pure virtual... this function should NOT be overridden by other SidePostprocessors
   */
  virtual Real computeQpResidual() { return 0; }
};

#endif
