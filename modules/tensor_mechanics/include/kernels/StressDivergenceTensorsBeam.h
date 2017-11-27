/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCETENSORSBEAM_H
#define STRESSDIVERGENCETENSORSBEAM_H

#include "Kernel.h"

// Forward Declarations
class StressDivergenceTensorsBeam;
class RankTwoTensor;

template <>
InputParameters validParams<StressDivergenceTensorsBeam>();

class StressDivergenceTensorsBeam : public Kernel
{
public:
  StressDivergenceTensorsBeam(const InputParameters & parameters);

protected:
  virtual void computeResidual();
  virtual Real computeQpResidual() { return 0.0; }
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);
  void computeDynamicTerms(std::vector<RealVectorValue> & global_force_res,
                           std::vector<RealVectorValue> & global_moment_res);
  void computeGlobalResidual(const MaterialProperty<RealVectorValue> * force,
                             const MaterialProperty<RealVectorValue> * moment,
                             const MaterialProperty<RankTwoTensor> * total_rotation,
                             std::vector<RealVectorValue> & global_force_res,
                             std::vector<RealVectorValue> & global_moment_res);

  std::string _base_name;

  const unsigned int _component;
  unsigned int _ndisp;
  std::vector<unsigned int> _disp_var;
  unsigned int _nrot;
  std::vector<unsigned int> _rot_var;
  const MaterialProperty<RealVectorValue> * _force;
  const MaterialProperty<RealVectorValue> * _moment;
  const MaterialProperty<RankTwoTensor> & _K11;
  const MaterialProperty<RankTwoTensor> & _K22;
  const MaterialProperty<RankTwoTensor> & _K22_cross;
  const MaterialProperty<RankTwoTensor> & _K21_cross;
  const MaterialProperty<RankTwoTensor> & _K21;
  const MaterialProperty<Real> & _original_length;
  const MaterialProperty<RankTwoTensor> * _total_rotation;
  const Real & _zeta;
  const Real & _alpha;
  const MaterialProperty<RealVectorValue> * _force_old;
  const MaterialProperty<RealVectorValue> * _moment_old;
  const MaterialProperty<RankTwoTensor> * _total_rotation_old;
  const MaterialProperty<RealVectorValue> * _force_older;
  const MaterialProperty<RealVectorValue> * _moment_older;
  const MaterialProperty<RankTwoTensor> * _total_rotation_older;
};

#endif // STRESSDIVERGENCETENSORSBEAM_H
