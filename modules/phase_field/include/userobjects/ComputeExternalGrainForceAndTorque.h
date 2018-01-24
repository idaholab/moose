/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEEXTERNALGRAINFORCEANDTORQUE_H
#define COMPUTEEXTERNALGRAINFORCEANDTORQUE_H

#include "ShapeElementUserObject.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class ComputeExternalGrainForceAndTorque;
class GrainTrackerInterface;

template <>
InputParameters validParams<ComputeExternalGrainForceAndTorque>();

/**
 * This class is here to get the force and torque acting on a grain
 */
class ComputeExternalGrainForceAndTorque
    : public DerivativeMaterialInterface<ShapeElementUserObject>,
      public GrainForceAndTorqueInterface
{
public:
  ComputeExternalGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void executeJacobian(unsigned int jvar);
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<Real> & getForceCJacobians() const;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const;

protected:
  unsigned int _qp;

  VariableName _c_name;
  unsigned int _c_var;
  /// material property that provides force density
  MaterialPropertyName _dF_name;
  const MaterialProperty<std::vector<RealGradient>> & _dF;
  /// material property that provides jacobian of force density with respect to c
  const MaterialProperty<std::vector<RealGradient>> & _dFdc;
  /// no. of order parameters
  const unsigned int _op_num;
  /// provide UserObject for calculating grain volumes and centers
  const GrainTrackerInterface & _grain_tracker;
  unsigned int _grain_num;
  unsigned int _ncomp;

  std::vector<unsigned int> _vals_var;
  std::vector<VariableName> _vals_name;
  std::vector<const MaterialProperty<std::vector<RealGradient>> *> _dFdeta;

  ///@{ providing grain forces, torques and their jacobians w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;

  /// vector storing grain force and torque values
  std::vector<Real> _force_torque_store;
  /// vector storing jacobian of grain force and torque values
  std::vector<Real> _force_torque_c_jacobian_store;
  std::vector<std::vector<Real>> _force_torque_eta_jacobian_store;

  unsigned int _total_dofs;
};

#endif // COMPUTEEXTERNALGRAINFORCEANDTORQUE_H
