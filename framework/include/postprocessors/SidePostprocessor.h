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

  virtual Real computeIntegral();

protected:
  Moose::Variable & _var;

  unsigned int _boundary_id;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;
  const std::vector<Point> & _normals;

  const Elem * & _current_elem;
  const Elem * & _current_side_elem;

  // unknown
  const VariableValue & _u;
  const VariableGrad & _grad_u;

  virtual Real computeQpIntegral() = 0;
};

#endif
