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

//Forward Declarations
class MaskedGrainForceAndTorque;

template<>
InputParameters validParams<MaskedGrainForceAndTorque>();

/**
 * This class is here to get the force and torque acting on a grain
 * from different userobjects and sum them all
 */
class MaskedGrainForceAndTorque :
    public GrainForceAndTorqueInterface,
    public GeneralUserObject
{
public:
  MaskedGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute(){};
  virtual void finalize(){};

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<RealGradient> & getForceDerivatives() const;
  virtual const std::vector<RealGradient> & getTorqueDerivatives() const;

protected:

  const GrainForceAndTorqueInterface & _grain_force_torque_input;
  const std::vector<RealGradient> & _grain_forces_input;
  const std::vector<RealGradient> & _grain_torques_input;
  const std::vector<RealGradient> & _grain_force_derivatives_input;
  const std::vector<RealGradient> & _grain_torque_derivatives_input;

  std::vector<unsigned int> _pinned_grains;
  unsigned int _num_pinned_grains;
  unsigned int _ncrys;

  ///@{ providing sum of all grain forces, torques & their derivatives
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<RealGradient> _force_derivatives;
  std::vector<RealGradient> _torque_derivatives;
  ///@}
};

#endif //MaskedGrainForceAndTorque_H
