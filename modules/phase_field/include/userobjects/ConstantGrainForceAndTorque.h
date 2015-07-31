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


#ifndef CONSTANTGRAINFORCEANDTORQUE_H
#define CONSTANTGRAINFORCEANDTORQUE_H

#include "GeneralUserObject.h"

//Forward Declarations
class ConstantGrainForceAndTorque;

template<>
InputParameters validParams<ConstantGrainForceAndTorque>();

/* This class is here to get the force and torque acting on a grain*/

class ConstantGrainForceAndTorque :  public GeneralUserObject
{
public:
  ConstantGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  const std::vector<RealGradient> & getForceValues() const;
  const std::vector<RealGradient> & getTorqueValues() const;
  const std::vector<RealGradient> & getForceDerivatives() const;
  const std::vector<RealGradient> & getTorqueDerivatives() const;

protected:

  /// Applied force on particles
  std::vector<Real> _F;
  std::vector<Real> _M;

  unsigned int _ncrys;
  unsigned int _ncomp;
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<RealGradient> _force_derivatives;
  std::vector<RealGradient> _torque_derivatives;
  /// vector storing grain force and torque values
  std::vector<Real> _force_torque_store;
  std::vector<Real> _force_torque_derivative_store;
};

#endif //CONSTANTGRAINFORCEANDTORQUE_H
