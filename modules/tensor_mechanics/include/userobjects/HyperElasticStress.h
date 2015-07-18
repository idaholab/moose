/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HYPERELASTICSTRESS_H
#define HYPERELASTICSTRESS_H

#include "GeneralUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

/**
 * This userobject class computes the PK2 stress
 * Returns the derivative of strain wrt right Cauchy-Green tensor
 * Currently uses Lagrangian strain
 * Computes other states, which depends on computed stress and strain
 */

class HyperElasticStress;

template<>
InputParameters validParams<HyperElasticStress>();

class HyperElasticStress : public GeneralUserObject
{
 public:
  HyperElasticStress(const std::string & name, InputParameters parameters);
  virtual ~HyperElasticStress() {}

  void initialize() {}
  void execute() {}
  void finalize() {}

  /**
   * Computes derivative of strain wrt right Cauchy-Green tensor
   */
  virtual RankFourTensor computeDstrainDce(const RankTwoTensor & ce) const;
  /**
   * Computes stress and associated states
   * Input: Elasticity tensor, right Cauchy-Green tensor, input states
   * Ouput: PK2 stress, Derivative of PK2 stress wrt right Cauchy-Green tensor, real valued states or derivative of states,
   *        RankTwoTensor valued states or Derivative of states
   */
  virtual void computePK2Stress(RankTwoTensor & pk2, RankFourTensor & dpk2_dce, std::vector<Real> & out_state_real, std::vector<RankTwoTensor> & out_state_ranktwotensor, const RankTwoTensor & ce, const RankFourTensor & elasticity_tensor, const std::vector<Real> & in_state) const;
  /**
   * Computes strain:
   * Input: Right Cauchy-Green tensor
   * Output: Strain (Lagrangian)
   */
  virtual RankTwoTensor computeStrain(const RankTwoTensor & ce) const;

  /**
   * Return the number of input states that are required in this object
   */
  unsigned int getNumStateIn() const;
  /**
   * Return the number of real valued states or derivatives that are calculated in this object
   */
  unsigned int getNumStateOutReal() const;
  /**
   * Return the number of ranktwotensor valued states or derivatives calculated in this object
   */
  unsigned int getNumStateOutRankTwoTensor() const;

 protected:
  ///Number of states required in this object
  unsigned int _num_in_state;
  ///Number of real valued states or state derivatives computed in this object
  unsigned int _num_out_state_real;
  ///Number of ranktwotensor valued states or derivative of states computed in this objectn
  unsigned int _num_out_state_ranktwotensor;

 private:
};

#endif //HYPERELASTICSTRESS_H
