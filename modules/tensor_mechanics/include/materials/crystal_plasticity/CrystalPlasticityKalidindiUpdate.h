//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CrystalPlasticityUpdate.h"

class CrystalPlasticityKalidindiUpdate;

/**
 * CrystalPlasticityKalidindiUpdate uses the multiplicative decomposition of the
 * deformation gradient and solves the PK2 stress residual equation at the
 * intermediate configuration to evolve the material state. The internal
 * variables are updated using an interative predictor-corrector algorithm.
 * Backward Euler integration rule is used for the rate equations.
 */

class CrystalPlasticityKalidindiUpdate : public CrystalPlasticityUpdate
{
public:
  static InputParameters validParams();

  CrystalPlasticityKalidindiUpdate(const InputParameters & parameters);

protected:
  /**
   * initializes the stateful properties such as
   * stress, plastic deformation gradient, slip system resistances, etc.
   */
  virtual void initQpStatefulProperties() override;

  /**
   * This virtual method is called to set the constitutive internal state variables
   * current value to the old property value for the start of the stress convergence
   * while loop. This class also calculates the constitutive slip system resistance
   * based on the values set for the constitutive state variables.
   */
  virtual void setInitialConstitutiveVariableValues() override;

  /*
   * This virtual method is called to calculate the total slip system slip
   * increment based on the constitutive model defined in the child class.
   * This method must be overwritten in the child class.
   */
  virtual void
  calculateConstitutiveEquivalentSlipIncrement(RankTwoTensor & equivalent_slip_increment,
                                               bool & error_tolerance) override;

  /*
   * This virtual method is called to find the derivative of the slip increment
   * with respect to the applied shear stress on the slip system based on the
   * constiutive model defined in the child class.  This method must be overwritten
   * in the child class.
   */
  virtual void calculateConstitutiveSlipDerivative(std::vector<Real> & dslip_dtau,
                                                   unsigned int slip_model_number = 0) override;

  /*
   * Finalizes the values of the state variables and slip system resistance
   * for the current timestep after convergence has been reached.
   */
  virtual void updateConstitutiveSlipSystemResistanceAndVariables(bool & error_tolerance) override;

  /*
   * Determines if the state variables, e.g. defect densities, have converged
   * by comparing the change in the values over the iteration period.
   */
  virtual bool areConstitutiveStateVariablesConverged() override;

  /**
   * Following the Constitutive model for slip system resistance as given in
   * Kalidindi, S.R., C.A. Bronkhorst, and L. Anand. Crystallographic texture
   * evolution in bulk deformation processing of FCC metals. Journal of the
   * Mechanics and Physics of Solids 40, no. 3 (1992): 537-569. Eqns 40 - 43.
   * The slip system resistant increment is calculated as
   * $\Delta g = \left| \Delta \gamma \cdot q^{\alpha \beta} \cdot h^{\beta} \right|$
   * and a convergence check is performed on the slip system resistance increment
   */
  void calculateSlipSystemResistance(bool & error_tolerance);

  ///@{Slip system resistance and slip increment variables
  MaterialProperty<std::vector<Real>> & _slip_system_resistance;
  const MaterialProperty<std::vector<Real>> & _slip_system_resistance_old;
  MaterialProperty<std::vector<Real>> & _slip_increment;
  MaterialProperty<std::vector<Real>> & _previous_it_slip_increment;
  MaterialProperty<std::vector<Real>> & _previous_it_resistance;
  ///@}

  ///@{Varibles used in the Kalidindi 1992 slip system resistance constiutive model
  const Real _r;
  const Real _h;
  const Real _tau_sat;
  const Real _gss_a;
  const Real _ao;
  const Real _xm;
  const Real _gss_initial;
  ///@}
};
