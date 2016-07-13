/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEGRAINFORCEANDTORQUE_H
#define COMPUTEGRAINFORCEANDTORQUE_H

#include "ElementUserObject.h"
#include "GrainForceAndTorqueInterface.h"
#include "DerivativeMaterialInterface.h"

//Forward Declarations
class ComputeGrainForceAndTorque;
class GrainTrackerInterface;

template<>
InputParameters validParams<ComputeGrainForceAndTorque>();

/**
 * This class is here to get the force and torque acting on a grain
 */
class ComputeGrainForceAndTorque :
    public ElementUserObject,
    public DerivativeMaterialPropertyNameInterface,
    public GrainForceAndTorqueInterface
{
public:
  ComputeGrainForceAndTorque(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual void finalize();
  virtual void threadJoin(const UserObject & y);

  virtual const std::vector<RealGradient> & getForceValues() const;
  virtual const std::vector<RealGradient> & getTorqueValues() const;
  virtual const std::vector<RealGradient> & getForceDerivatives() const;
  virtual const std::vector<RealGradient> & getTorqueDerivatives() const;

protected:
  unsigned int _qp;

  VariableName _c_name;
  /// material property that provides force density
  const MaterialProperty<std::vector<RealGradient> > & _dF;
  MaterialPropertyName _dF_name;
  /// material property that provides derivative of force density with respect to c
  const MaterialProperty<std::vector<RealGradient> > & _dFdc;
  /// provide UserObject for calculating grain volumes and centers
  const GrainTrackerInterface & _grain_tracker;
  unsigned int _ncrys;
  unsigned int _ncomp;
  ///@{ providing grain forces, torques and their derivatives w. r. t c
  std::vector<RealGradient> _force_values;
  std::vector<RealGradient> _torque_values;
  std::vector<RealGradient> _force_derivatives;
  std::vector<RealGradient> _torque_derivatives;
  ///@}
  /// vector storing grain force and torque values
  std::vector<Real> _force_torque_store;
  /// vector storing derivative of grain force and torque values
  std::vector<Real> _force_torque_derivative_store;
};

#endif //COMPUTEGRAINFORCEANDTORQUE_H
