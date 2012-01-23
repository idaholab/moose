/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SIDEPOSTPROCESSOR_H
#define SIDEPOSTPROCESSOR_H

#include "Postprocessor.h"
#include "MooseVariable.h"
#include "MaterialPropertyInterface.h"

//Forward Declarations
class SidePostprocessor;

template<>
InputParameters validParams<SidePostprocessor>();

class SidePostprocessor :
  public Postprocessor,
  public MaterialPropertyInterface
{
public:
  SidePostprocessor(const std::string & name, InputParameters parameters);

  const std::vector<unsigned int> & boundaryIDs() { return _boundary_ids; }

  virtual Real computeIntegral();

protected:
  MooseVariable & _var;

  std::vector<unsigned int> _boundary_ids;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;
  const std::vector<Real> & _coord;
  const std::vector<Point> & _normals;

  const Elem * & _current_elem;
  const Elem * & _current_side_elem;
  const Real & _current_side_volume;

  // unknown
  const VariableValue & _u;
  const VariableGradient & _grad_u;

  virtual Real computeQpIntegral() = 0;
};

#endif
