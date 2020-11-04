#pragma once

#include "FormFunction.h"

class ObjectiveGradientMinimize : public FormFunction
{
public:
  static InputParameters validParams();
  ObjectiveGradientMinimize(const InputParameters & parameters);

  virtual Real computeObjective() override;
  virtual void computeGradient() override;

private:
  /// number of data values
  unsigned int _data_size;
  /// vector of simulation data
  std::vector<const PostprocessorValue *> _data_computed;
  /// vector of measured data
  const std::vector<Real> & _data_target;
  /// misfit at data points
  std::vector<Real> _data_misfit;
  /// VPP to send misfit data to for adjoint
  OptimizationParameterVectorPostprocessor & _adjoint_vpp;
  /// vector of simulation data
  std::vector<const PostprocessorValue *> _adjoint_data_computed;
};
