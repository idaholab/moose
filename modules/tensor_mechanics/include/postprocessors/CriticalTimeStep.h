//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementPostprocessor.h"
#include "GuaranteeConsumer.h"
#include "ComputeStressBase.h"

/**
 * Compute the critical time step for an explicit integration scheme by inferring an
 * effective_stiffness from element classes and density from the user.
 */

// Forward Declarations

class CriticalTimeStep : public ElementPostprocessor, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  CriticalTimeStep(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void initialSetup() override;

  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Density of the material
  const MaterialProperty<Real> & _material_density;

  /// Added density due to mass scaling (zero if no scaling is selected or applied)
  const MaterialProperty<Real> * _density_scaling;

  /// Effective stiffness of element: function of material properties and element size
  const MaterialProperty<Real> & _effective_stiffness;

  /// User defined factor to be multiplied to the critical time step
  const Real & _factor;

  /// Critical time step for explicit solver
  Real _critical_time;
};
