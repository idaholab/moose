//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACGrGrPoly.h"

// Forward Declarations

/**
 * This kernel calculates the residual for grain growth for a single phase,
 * polycrystal system using the linearized interface grain growth model.
 */
class ACGrGrPolyLinearizedInterface : public ACGrGrPoly
{
public:
  static InputParameters validParams();

  ACGrGrPolyLinearizedInterface(const InputParameters & parameters);

protected:
  virtual Real assignThisOp();
  virtual std::vector<Real> assignOtherOps();
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const std::vector<MaterialPropertyName> _other_op_names;
  const unsigned int _num_ops;
  const MaterialProperty<Real> & _gamma;
  const MaterialProperty<Real> & _op;
  const MaterialProperty<Real> & _dopdphi;

  std::vector<const MaterialProperty<Real> *> _opj;
  std::vector<const MaterialProperty<Real> *> _dopjdarg;
};
