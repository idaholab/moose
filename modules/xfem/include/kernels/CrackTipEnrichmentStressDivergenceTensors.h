//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ALEKernel.h"
#include "ADKernel.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "CrackFrontDefinition.h"
#include "EnrichmentFunctionCalculation.h"

template <bool is_ad>
using CrackTipEnrichmentStressDivergenceTensorsParent =
    typename std::conditional<is_ad, ADKernel, ALEKernel>::type;

/**
 * CrackTipEnrichmentStressDivergenceTensors implements the residual and jacobian for enrichment
 * displacement variables. Both non-AD (is_ad=false, default) and AD (is_ad=true) versions are
 * provided by this single template class. The non-AD version computes the Jacobian analytically
 * using _Jacobian_mult; the AD version computes it automatically via MOOSE's AD system.
 */
template <bool is_ad>
class CrackTipEnrichmentStressDivergenceTensorsTempl
  : public CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>,
    public EnrichmentFunctionCalculation
{
public:
  static InputParameters validParams();

  CrackTipEnrichmentStressDivergenceTensorsTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const std::string _base_name;

  const GenericMaterialProperty<RankTwoTensor, is_ad> & _stress;
  /// Only used for the non-AD path; nullptr when is_ad=true
  const MaterialProperty<RankFourTensor> * _Jacobian_mult;

  const GenericMaterialProperty<RankTwoTensor, is_ad> * _deformation_gradient;
  const MaterialProperty<RankTwoTensor> * _deformation_gradient_old;
  const GenericMaterialProperty<RankTwoTensor, is_ad> * _rotation_increment;

  /// displacement components
  const unsigned int _component;
  /// enrichment function components
  const unsigned int _enrichment_component;

  /// Coupled enrichment displacement variables
  unsigned int _nenrich_disp;
  std::vector<unsigned int> _enrich_disp_var;

  /// Coupled displacement variables
  unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;

  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_qp;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_i;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_j;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_test;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_grad_test;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_phi;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_grad_phi;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_q_point;
  using CrackTipEnrichmentStressDivergenceTensorsParent<is_ad>::_current_elem;

private:
  /// enrichment function value
  std::vector<Real> _B;
  /// derivatives of enrichment function respect to global coordinate
  std::vector<RealVectorValue> _dBX;
  /// derivatives of enrichment function respect to crack front coordinate
  std::vector<RealVectorValue> _dBx;
  /// enrichment function at node I
  std::vector<Real> _BI;
  /// enrichment function at node J
  std::vector<Real> _BJ;
};

using CrackTipEnrichmentStressDivergenceTensors =
    CrackTipEnrichmentStressDivergenceTensorsTempl<false>;
using ADCrackTipEnrichmentStressDivergenceTensors =
    CrackTipEnrichmentStressDivergenceTensorsTempl<true>;
