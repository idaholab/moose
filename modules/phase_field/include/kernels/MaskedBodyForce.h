//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BodyForce.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class Function;

/**
 * This kernel creates a body force that is modified by a mask defined
 * as a material. Common uses of this would be to turn off or change the
 * body force in certain regions of the mesh.
 */

class MaskedBodyForce : public DerivativeMaterialInterface<JvarMapKernelInterface<BodyForce>>
{
public:
  static InputParameters validParams();

  MaskedBodyForce(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _mask;

  /// name of the nonlinear variable (needed to retrieve the derivative material properties)
  VariableName _v_name;

  /// derivative of the mask wrt the kernel's nonlinear variable
  const MaterialProperty<Real> & _dmaskdv;

  ///  Reaction rate derivatives w.r.t. other coupled variables
  std::vector<const MaterialProperty<Real> *> _dmaskdarg;
};
