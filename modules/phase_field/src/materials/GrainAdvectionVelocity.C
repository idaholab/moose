#include "GrainAdvectionVelocity.h"

template<>
InputParameters validParams<GrainAdvectionVelocity>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculation the advection velocity of grain due to rigid vody translation and rotation");
  params.addCoupledVar("etas", "Array of other coupled order parameters");
  params.addParam<Real>("translation_constant",500, "constant value characterizing grain translation");
  params.addParam<Real>("rotation_constant",1.0, "constant value characterizing grain rotation");
  params.addParam<UserObjectName>("grain_data","userobject for getting volume and center of mass of grains");
  params.addParam<UserObjectName>("grain_force","userobject for getting force and torque acting on grains");
  return params;
}

GrainAdvectionVelocity::GrainAdvectionVelocity(const std::string & name, InputParameters parameters) :
   Material(name,parameters),
   _grain_data(getUserObject<ComputeGrainCenterUserObject>("grain_data")),
   _grain_volumes(_grain_data.getGrainVolumes()),
   _grain_centers(_grain_data.getGrainCenters()),
   _grain_force_torque(getUserObject<ComputeGrainForceAndTorque>("grain_force")),
   _grain_forces(_grain_force_torque.getForceValues()),
   _grain_torques(_grain_force_torque.getTorqueValues()),
   _mt(getParam<Real>("translation_constant")),
   _mr(getParam<Real>("rotation_constant")),
   _ncrys(_grain_forces.size()),
   //_ncrys(coupledComponents("etas")), //determine number of grains from the number of names passed in.  Note this is the actual number -1
   _vals(_ncrys), //Size variable arrays
   _velocity_advection(declareProperty<std::vector<RealGradient> >("advection_velocity"))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
    _vals[i] = &coupledValue("etas", i);
}

void
GrainAdvectionVelocity::timestepSetup()
{
  for (_qp = 0; _qp < _velocity_advection.size(); ++_qp)
  _velocity_advection[_qp].resize(_ncrys);
}

void
GrainAdvectionVelocity::computeQpProperties()
{
  std::vector<RealGradient> _velocity_translation(_ncrys);
  std::vector<RealGradient> _velocity_rotation(_ncrys);
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _velocity_translation[i] = _mt/_grain_volumes[i]*((*_vals[i])[_qp]*_grain_forces[i]);
    _velocity_rotation[i] = _mr*(_grain_torques[i].cross(_q_point[_qp]-_grain_centers[i]))*(*_vals[i])[_qp]/_grain_volumes[i];
    _velocity_advection[_qp][i] = _velocity_translation[i] + _velocity_rotation[i];
  }
}
