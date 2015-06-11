/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef STRESSDIVERGENCEPFFRACTENSORS_H
#define STRESSDIVERGENCEPFFRACTENSORS_H

#include "StressDivergenceTensors.h"

/**
 * This class computes the off-diagonal Jacobian component of stress divergence residual system
 * Contribution from damage order parameter c
 * Residual calculated in StressDivergenceTensors
 * Useful if user wants to add the off diagonal Jacobian term
 */

class StressDivergencePFFracTensors;

template<>
InputParameters validParams<StressDivergencePFFracTensors>();

class StressDivergencePFFracTensors : public StressDivergenceTensors
{
public:
  StressDivergencePFFracTensors(const std::string & name, InputParameters parameters);

protected:

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<RankTwoTensor> * _d_stress_dc;
  std::string _pff_jac_prop_name;

  const bool _c_coupled;
  const unsigned int _c_var;

private:

};

#endif //STRESSDIVERGENCEPFFRACTENSORS_H
