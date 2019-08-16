/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AnisoTwoGrainEulerAngleProvider.h"
#include "FEProblemBase.h"

registerMooseObject("MarmotApp", AnisoTwoGrainEulerAngleProvider);

template <>
InputParameters
validParams<AnisoTwoGrainEulerAngleProvider>()
{
  InputParameters params = validParams<EulerAngleProvider>();
  params.addClassDescription("Provides Euler angles for two grains based on simulation time.");
  params.addRequiredParam<unsigned int>(
      "axis_type", "Which axis type we are rotating about. 0 = <100>, 1 = <110>, 2 = <111>");
  return params;
}

AnisoTwoGrainEulerAngleProvider::AnisoTwoGrainEulerAngleProvider(const InputParameters & parameters)
  : EulerAngleProvider(parameters),
    _angles(2),
    _axis_type(getParam<unsigned int>("axis_type")),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  // Variables needed to start everything.
  EulerAngles initial_euler;
  EulerAngles initial_euler2;
  const RealVectorValue axis100(1, 0, 0);

  // Start at no rotation.
  initial_euler.phi1 = 0;
  initial_euler.phi2 = 0;
  initial_euler.Phi = 0;
  // Start at no rotation.
  initial_euler2.phi1 = 0;
  initial_euler2.phi2 = 0;
  initial_euler2.Phi = 0;
  createRotationAxis();

  // Rotate [1 0 0] to our rotation axis
  _rotate_to_axis = RotationMatrix::rotVec1ToVec2(_rotation_axis, axis100);

  // Put grain1 and grain2 matrices into proper coordinates
  _gb1_orientation_matrix = _rotate_to_axis * RotationTensor(initial_euler);

  _gb2_orientation_matrix = _rotate_to_axis * RotationTensor(initial_euler2);
  _rotation_matrix_gb1 = RotationTensor(RotationTensor::XAXIS, _fe_problem.dt() / 2.0);
  _rotation_matrix_gb2 = RotationTensor(RotationTensor::XAXIS, -_fe_problem.dt() / 2.0);

  _angles[0] = initial_euler;
  _angles[1] = initial_euler2;
}

void
AnisoTwoGrainEulerAngleProvider::execute()
{
  updateEulerAngles();
}

const EulerAngles &
AnisoTwoGrainEulerAngleProvider::getEulerAngles(unsigned int i) const
{
  mooseAssert(i < getGrainNum(), "Requesting Euler angles for an invalid grain id");
  return _angles[i];
}

unsigned int
AnisoTwoGrainEulerAngleProvider::getGrainNum() const
{
  mooseAssert(_angles.size() == 2, "Too many grains for AnisoTwoGrainEulerAngleProvider");
  return _angles.size();
}

void
AnisoTwoGrainEulerAngleProvider::updateEulerAngles()
{
  // Update just in case dt changes for some reason.
  _rotation_matrix_gb1 = RotationTensor(RotationTensor::XAXIS, _fe_problem.dt() / 2.0);
  _rotation_matrix_gb2 = RotationTensor(RotationTensor::XAXIS, -_fe_problem.dt() / 2.0);

  _gb1_orientation_matrix = _rotation_matrix_gb1 * _gb1_orientation_matrix;
  _gb2_orientation_matrix = _rotation_matrix_gb2 * _gb2_orientation_matrix;

  // Store the euler angles. The input file takes care of whether it's twist or tilt
  _angles[0] = rotationMatrixToEuler(_gb1_orientation_matrix);
  _angles[1] = rotationMatrixToEuler(_gb2_orientation_matrix);
}

void
AnisoTwoGrainEulerAngleProvider::createRotationAxis()
{
  if (_axis_type == 0)
    _rotation_axis = RealVectorValue(1, 0, 0);
  else if (_axis_type == 1)
    _rotation_axis = RealVectorValue(1, 1, 0) / std::sqrt(2.0);
  else if (_axis_type == 2)
    _rotation_axis = RealVectorValue(1, 1, 1) / std::sqrt(3.0);
  else
    mooseError("Undefined axis type.");
}

EulerAngles
AnisoTwoGrainEulerAngleProvider::rotationMatrixToEuler(RealTensorValue m)
{
  EulerAngles m_euler;

  if (std::abs(m(2, 2)) == 1)
  {
    m_euler.phi1 = std::atan2(m(0, 1), m(0, 0));
    m_euler.Phi = (libMesh::pi / 2) * (1 - m(2, 2));
    m_euler.phi2 = 0;
  }
  else
  {
    Real K = 1 / (std::sqrt(1 - (m(2, 2) * m(2, 2))));
    m_euler.phi1 = std::atan2(K * m(2, 0), -K * m(2, 1));
    m_euler.Phi = std::acos(m(2, 2));
    m_euler.phi2 = std::atan2(K * m(0, 2), K * m(1, 2));
  }

  // Convert to degrees
  m_euler.phi1 = m_euler.phi1 * 180.0 / libMesh::pi;
  m_euler.Phi = m_euler.Phi * 180.0 / libMesh::pi;
  m_euler.phi2 = m_euler.phi2 * 180.0 / libMesh::pi;

  return m_euler;
}
