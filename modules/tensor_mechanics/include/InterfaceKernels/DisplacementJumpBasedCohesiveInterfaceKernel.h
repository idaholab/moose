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

#ifndef DISPLACMENTJUMPBASEDCOHESIVEINTERFACEKERNEL_H
#define DISPLACMENTJUMPBASEDCOHESIVEINTERFACEKERNEL_H

#include "InterfaceKernel.h"
#include "RankTwoTensor.h"

/// Forward Declarations
class DisplacementJumpBasedCohesiveInterfaceKernel;

template <>
InputParameters validParams<DisplacementJumpBasedCohesiveInterfaceKernel>();

/// DG kernel implementing a basic for a 3D traction sepration law based on
/// the displacement jump. This kernel operates only on a signle displacement
/// compenent. One kernel is needed for each dispalcement jump component
class DisplacementJumpBasedCohesiveInterfaceKernel : public InterfaceKernel
{
public:
  DisplacementJumpBasedCohesiveInterfaceKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// the displacement component this kernel is operating on (0=x, 1=y, 2 =z)
  const unsigned int _disp_index;

  /// coupled displacement componenets values
  const VariableValue & _disp_1;
  const VariableValue & _disp_1_neighbor;
  const VariableValue & _disp_2;
  const VariableValue & _disp_2_neighbor;

  /// coupled displacement componenets variables ID
  unsigned int _disp_1_var;
  unsigned int _disp_1_neighbor_var;
  unsigned int _disp_2_var;
  unsigned int _disp_2_neighbor_var;

  /// variables containg the names of the material properties representing the
  /// reidual's and jacobian's coefficients. Derivates are assumed to be taken
  /// wrt to the displacement jump.
  const std::string _residual;
  const std::string _jacobian;

  // values of the residual's and jacobian's cofficients
  const MaterialProperty<RealVectorValue> * _ResidualMP;
  const MaterialProperty<RankTwoTensor> * _JacobianMP;
};

#endif // DISPLACMENTJUMPBASEDCOHESIVEINTERFACEKERNEL_H
