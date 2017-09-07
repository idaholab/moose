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

#ifndef INTERNALSIDEJUMP_H
#define INTERNALSIDEJUMP_H

#include "InternalSidePostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class InternalSideJump;

template <>
InputParameters validParams<InternalSideJump>();

class InternalSideJump : public InternalSidePostprocessor, public MooseVariableInterface
{
public:
  InternalSideJump(const InputParameters & parameters);

  virtual PostprocessorValue getValue() override;
  virtual void execute() override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  const Real & _current_elem_volume;
  const Real & _current_neighbor_volume;
  const DenseVector<Number> & _sln_dofs;
  const DenseVector<Number> & _sln_dofs_neig;
  Real _integral_value;
};

#endif
