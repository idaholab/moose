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

#ifndef DASHPOTBC_H
#define DASHPOTBC_H

#include "IntegratedBC.h"


//Forward Declarations
class DashpotBC;

template<>
InputParameters validParams<DashpotBC>();

/**
 * Implements a simple constant Dashpot BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class DashpotBC : public IntegratedBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  DashpotBC(const std::string & name, InputParameters parameters);
  
virtual ~DashpotBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  
private:
  unsigned int _component;
  Real _coefficient;

  unsigned int _disp_x_var;
  unsigned int _disp_y_var;
  unsigned int _disp_z_var;
  
  VariableValue & _disp_x_dot;
  VariableValue & _disp_y_dot;
  VariableValue & _disp_z_dot;
};

#endif //DASHPOTBC_H
