
#ifndef CHANNELGRADIENTBC_H
#define CHANNELGRADIENTBC_H

#include "IntegratedBC.h"

class ChannelGradientBC;

template <>
InputParameters validParams<ChannelGradientBC>();

class ChannelGradientBC : public IntegratedBC
{
public:
  ChannelGradientBC(const InputParameters & parameters);

protected:
  virtual Real getGradient();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MooseEnum _axis;
  const VectorPostprocessorValue & _channel_gradient_axis_coordinate;
  const VectorPostprocessorValue & _channel_gradient_value;
  const MaterialProperty<Real> & _h;
};

#endif // CHANNELGRADIENTBC_H
