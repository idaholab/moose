//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityUOBase.h"
#include "RankTwoTensor.h"

/**
 * Crystal plasticity slip rate userobject class
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */
class CrystalPlasticitySlipRate : public CrystalPlasticityUOBase
{
public:
  static InputParameters validParams();

  CrystalPlasticitySlipRate(const InputParameters & parameters);

  virtual void calcFlowDirection(unsigned int qp,
                                 std::vector<RankTwoTensor> & flow_direction) const = 0;
  virtual bool calcSlipRate(unsigned qp, Real dt, std::vector<Real> & val) const = 0;
  virtual bool calcSlipRateDerivative(unsigned qp, Real dt, std::vector<Real> & val) const = 0;

protected:
  virtual void getSlipSystems();

  virtual void readFileFlowRateParams();

  virtual void getFlowRateParams();

  /// Number of slip system specific properties provided in the file containing slip system normals and directions
  unsigned int _num_slip_sys_props;

  std::vector<Real> _flowprops;

  /// File should contain slip plane normal and direction.
  std::string _slip_sys_file_name;

  /**
   * File should contain values of the flow rate equation parameters.
   * Values for every slip system must be provided.
   * Should have the same order of slip systens as in slip_sys_file.
   * The option of reading all the properties from .i is still present.
   */
  std::string _slip_sys_flow_prop_file_name;

  /// Number of slip system flow rate parameters
  unsigned int _num_slip_sys_flowrate_props;

  /// Slip increment tolerance
  Real _slip_incr_tol;

  DenseVector<Real> _mo;
  DenseVector<Real> _no;

  /// Crystal rotation
  const MaterialProperty<RankTwoTensor> & _crysrot;
};
