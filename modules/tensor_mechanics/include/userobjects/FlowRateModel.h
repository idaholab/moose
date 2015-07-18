/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FLOWRATEMODEL_H
#define FLOWRATEMODEL_H

#include "GeneralUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * This userobject class is for  hyper elastic visco plastic model and computes:
 * The flow rate using power law
 * The flow direction based on J2
 * The internal variable equivalent plastic strain and its derivative with flow rate
 * The derivative of flow rate with PK2 stress
 * The derivative of flow rate with internal variable
 */

class FlowRateModel;

template<>
InputParameters validParams<FlowRateModel>();

class FlowRateModel : public GeneralUserObject
{
public:
  FlowRateModel(const std::string & name, InputParameters parameters);
  virtual ~FlowRateModel() {}

  void initialize() {}
  void execute() {}
  void finalize() {}

  /**
   * This function computes the flow rate
   * Input: PK2 stress, Elastic right Cauchy Green tensor
   * Input: Internal variables, starting index in the internal variable vector and number of internal variables from starting index (size)
   * Ouput: Flow rate, error  = false - if calculated flow rate exceeds a tolerance
   */
  virtual bool computeFlowRate(Real & flow_rate, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const;
  /**
   * This function computes the flow direction
   * Input: PK2 stress, Elastic right Cauchy Green tensor
   * Input: Internal variables, starting index in the internal variable vector and number of internal variables from starting index (size)
   * Ouput: Flow direction
   */
  virtual bool computeFlowDirection(RankTwoTensor & flow_dirn, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const;
  /**
   * This function computes the derivative of flow rate wrt PK2 stress
   * Input: PK2 stress, Elastic right Cauchy Green tensor
   * Input: Internal variables, starting index in the internal variable vector and number of internal variables from starting index (size)
   * Ouput: Flow direction
   */
  virtual bool computeDflowrateDstress(RankTwoTensor & dflowrate_dstress, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const;
  /**
   * This function computes the derivative of flow rate wrt internal variables
   * Input: PK2 stress, Elastic right Cauchy Green tensor
   * Input: Internal variables, starting index in the internal variable vector and number of internal variables from starting index (size)
   * Ouput: dflowrate_dq (derivative of flow rate wrt internal variables)
   */
  virtual bool computeDflowrateDinternalvar(std::vector<Real> & dflowrate_dq, const RankTwoTensor & pk2, const RankTwoTensor & ce, const std::vector<Real> & internal_var, const unsigned int start_index, const unsigned int size) const;
  /**
   * This function updates the internal variables and derivative of internal variables wrt flowrate
   * Input: Flow rate (flow_rate), time increment (dt), Old internal variables (q_old)
   * Input: Starting index in the internal variable vector and number of internal variables from starting index (size)
   * Ouput: Current internal variables (_q), derivative of internal variables wrt flowrate (dq_dflowrate)
   */
  virtual bool updateInternalVar(std::vector<Real> & q, std::vector<Real> & dq_dflowrate, const Real flow_rate, const Real dt, const std::vector<Real> & q_old, const unsigned int start_index, const unsigned int size) const;
  /**
   * Returns the number of internal variables associated with the object
   */
  unsigned int numInternalVar() const;
  /**
   * Returns the deviatoric stress
   */
  RankTwoTensor computePK2Deviatoric(const RankTwoTensor & pk2, const RankTwoTensor & ce) const;
  /**
   * Returns the equivalent deviatoric stress
   */
  Real computeEqvStress(const RankTwoTensor & pk2_dev, const RankTwoTensor & ce) const;

protected:
  ///Number of internal variables associated with this object
  unsigned int _num_internal_var;
  ///Reference flow rate
  Real _ref_flow_rate;
  ///Flow rate exponent
  Real _flow_rate_exponent;
  ///Allowable flow rate
  Real _flow_rate_tol;
  ///Flow stress user object
  const TensorMechanicsHardeningModel & _flow_stress_uo;

private:
};

#endif //FLOWRATEMODEL_H
