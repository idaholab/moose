//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SlopeReconstruction1DInterface.h"

class VaporMixtureFluidProperties;

/**
 * Computes reconstructed solution values for FlowModelGasMix.
 */
class SlopeReconstructionGasMixMaterial : public Material,
                                          public SlopeReconstruction1DInterface<true>
{
public:
  static InputParameters validParams();

  SlopeReconstructionGasMixMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual std::vector<ADReal> computeElementPrimitiveVariables(const Elem * elem) const override;

  /// Cross-sectional area, piecewise constant
  const ADVariableValue & _A_avg;
  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;

  // piecewise constant conserved variable values
  const ADVariableValue & _xirhoA_avg;
  const ADVariableValue & _rhoA_avg;
  const ADVariableValue & _rhouA_avg;
  const ADVariableValue & _rhoEA_avg;

  // piecewise constant conserved variables
  MooseVariable * _A_var;
  MooseVariable * _xirhoA_var;
  MooseVariable * _rhoA_var;
  MooseVariable * _rhouA_var;
  MooseVariable * _rhoEA_var;

  /// Flow channel direction
  const MaterialProperty<RealVectorValue> & _dir;

  // reconstructed variable values
  ADMaterialProperty<Real> & _xirhoA;
  ADMaterialProperty<Real> & _rhoA;
  ADMaterialProperty<Real> & _rhouA;
  ADMaterialProperty<Real> & _rhoEA;

  /// fluid properties user object
  const VaporMixtureFluidProperties & _fp;

  /// Solution variables vector
  std::vector<MooseVariable *> _U_vars;
};
