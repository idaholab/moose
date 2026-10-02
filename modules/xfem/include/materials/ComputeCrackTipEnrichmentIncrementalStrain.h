//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ComputeIncrementalStrainBase.h"
#include "ADComputeIncrementalStrainBase.h"
#include "Material.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationTensor.h"
#include "Assembly.h"
#include "CrackFrontDefinition.h"
#include "EnrichmentFunctionCalculation.h"

template <bool is_ad>
using ComputeIncrementalStrainBaseParent = typename std::
    conditional<is_ad, ADComputeIncrementalStrainBase, ComputeIncrementalStrainBase>::type;

template <bool is_ad>
using RealParent = typename std::conditional<is_ad, ADReal, Real>::type;

template <bool is_ad>
using RealVectorValueParent =
    typename std::conditional<is_ad, ADRealVectorValue, RealVectorValue>::type;

/**
 * ComputeIncrementalStrain defines a strain increment and rotation increment (=1), for small
 * strains.
 */
template <bool is_ad>
class ComputeCrackTipEnrichmentIncrementalStrainTempl
  : public ComputeIncrementalStrainBaseParent<is_ad>,
    public EnrichmentFunctionCalculation
{
public:
  static InputParameters validParams();

  ComputeCrackTipEnrichmentIncrementalStrainTempl(const InputParameters & parameters);
  virtual ~ComputeCrackTipEnrichmentIncrementalStrainTempl() {}
  virtual void computeProperties() override;

protected:
  /// enrichment displacement
  std::vector<RealParent<is_ad>> _enrich_disp;

  /// gradient of enrichment displacement
  std::vector<RealVectorValueParent<is_ad>> _grad_enrich_disp;
  std::vector<RealVectorValueParent<is_ad>> _grad_enrich_disp_old;

  /// enrichment displacement variables
  std::vector<std::vector<MooseVariableFEBase *>> _enrich_variable;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  const MaterialProperty<RankTwoTensor> & _mechanical_strain_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;
  using ComputeIncrementalStrainBaseParent<is_ad>::_fe_problem;
  using ComputeIncrementalStrainBaseParent<is_ad>::_assembly;
  using ComputeIncrementalStrainBaseParent<is_ad>::_ndisp;
  using ComputeIncrementalStrainBaseParent<is_ad>::_current_elem;
  using ComputeIncrementalStrainBaseParent<is_ad>::_qrule;
  using ComputeIncrementalStrainBaseParent<is_ad>::isBoundaryMaterial;
  using ComputeIncrementalStrainBaseParent<is_ad>::_current_side;
  using ComputeIncrementalStrainBaseParent<is_ad>::_qp;
  using ComputeIncrementalStrainBaseParent<is_ad>::_q_point;
  using ComputeIncrementalStrainBaseParent<is_ad>::_grad_disp;
  using ComputeIncrementalStrainBaseParent<is_ad>::_grad_disp_old;
  using ComputeIncrementalStrainBaseParent<is_ad>::_strain_increment;
  using ComputeIncrementalStrainBaseParent<is_ad>::_total_strain;
  using ComputeIncrementalStrainBaseParent<is_ad>::_dt;
  using ComputeIncrementalStrainBaseParent<is_ad>::_strain_rate;
  using ComputeIncrementalStrainBaseParent<is_ad>::_mechanical_strain;
  using ComputeIncrementalStrainBaseParent<is_ad>::_rotation_increment;

private:
  /// enrichment function value
  std::vector<Real> _B;
  /// derivatives of enrichment function respect to global cooridnate
  std::vector<RealVectorValue> _dBX;
  /// derivatives of enrichment function respect to crack front cooridnate
  std::vector<RealVectorValue> _dBx;
  /// enrichment function at node I
  std::vector<std::vector<Real>> _BI;
  /// shape function
  const std::vector<std::vector<Real>> * _fe_phi;
  /// gradient of shape function
  const std::vector<std::vector<RealGradient>> * _fe_dphi;
  NonlinearSystem * _nl;
  const NumericVector<Number> * _sln;

  using RealParentType = RealParent<is_ad>;
  using RealParentVectorType = RealVectorValueParent<is_ad>;
};

using ComputeCrackTipEnrichmentIncrementalStrain =
    ComputeCrackTipEnrichmentIncrementalStrainTempl<false>;
using ADComputeCrackTipEnrichmentIncrementalStrain =
    ComputeCrackTipEnrichmentIncrementalStrainTempl<true>;
