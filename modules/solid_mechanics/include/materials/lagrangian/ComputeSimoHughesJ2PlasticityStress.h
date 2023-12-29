//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStressPK1.h"
#include "GuaranteeConsumer.h"
#include "ElasticityTensorTools.h"
#include "SingleVariableReturnMappingSolution.h"
#include "Function.h"
#include "DerivativeMaterialInterface.h"

/* This class implements the Simo-Hughes style J2 plasticity */
class ComputeSimoHughesJ2PlasticityStress
  : public DerivativeMaterialInterface<ComputeLagrangianStressPK1>,
    public GuaranteeConsumer,
    public SingleVariableReturnMappingSolution
{
public:
  static InputParameters validParams();

  ComputeSimoHughesJ2PlasticityStress(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpPK1Stress() override;

  /// @{ The return mapping residual and derivative
  virtual Real computeReferenceResidual(const Real & effective_trial_stress,
                                        const Real & scalar) override;
  virtual Real computeResidual(const Real & effective_trial_stress, const Real & scalar) override;
  virtual Real computeDerivative(const Real & effective_trial_stress, const Real & scalar) override;
  virtual void
  preStep(const Real & scalar_old, const Real & residual, const Real & jacobian) override;
  /// @}

  const MaterialPropertyName _elasticity_tensor_name;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  const MaterialProperty<RankTwoTensor> & _F_old;
  const std::string _ep_name;
  MaterialProperty<Real> & _ep;
  const MaterialProperty<Real> & _ep_old;
  MaterialProperty<RankTwoTensor> & _be;
  const MaterialProperty<RankTwoTensor> & _be_old;
  MaterialProperty<RankTwoTensor> & _Np;

  MaterialBase * _flow_stress_material;
  const std::string _flow_stress_name;
  const MaterialProperty<Real> & _H;
  const MaterialProperty<Real> & _dH;
  const MaterialProperty<Real> & _d2H;

private:
  /// @{ Helper (dummy) variables for iteratively updating the consistant tangent during return mapping
  RankFourTensor _d_be_d_F;
  RankFourTensor _d_n_d_be;
  RankTwoTensor _d_deltaep_d_betr;
  RankTwoTensor _d_R_d_betr;
  RankTwoTensor _d_J_d_betr;
  /// @}
};
