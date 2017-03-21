/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MaskedGrainForceAndTorque_H
#define MaskedGrainForceAndTorque_H

#include "GrainForceAndTorqueInterface.h"
#include "GeneralUserObject.h"

// Forward Declarations
class MaskedGrainForceAndTorque;

template <>
InputParameters validParams<MaskedGrainForceAndTorque>();

/**
 * This class is here to get the force and torque acting on a grain
 * from different userobjects and sum them all
 */
class MaskedGrainForceAndTorque : public GrainForceAndTorqueInterface, public GeneralUserObject
{
public:
  MaskedGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute(){};
  virtual void finalize(){};

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<Real> & getForceCJacobians() const;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const;

protected:
  const GrainForceAndTorqueInterface & _grain_force_torque_input;
  const std::vector<RealGradient> & _grain_forces_input;
  const std::vector<RealGradient> & _grain_torques_input;
  const std::vector<Real> & _grain_force_c_jacobians_input;
  const std::vector<std::vector<Real>> & _grain_force_eta_jacobians_input;

  std::vector<unsigned int> _pinned_grains;
  unsigned int _num_pinned_grains;
  unsigned int _grain_num;

  ///@{ providing grain forces, torques and their jacobians w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<Real> _c_jacobians;
  std::vector<std::vector<Real>> _eta_jacobians;
  ///@}
};

#endif // MaskedGrainForceAndTorque_H
