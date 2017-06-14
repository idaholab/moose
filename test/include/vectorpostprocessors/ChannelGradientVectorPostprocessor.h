#ifndef CHANNELGRADIENTVECTORPOSTPROCESSOR_H
#define CHANNELGRADIENTVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ChannelGradientVectorPostprocessor;

template <>
InputParameters validParams<ChannelGradientVectorPostprocessor>();

/**
 *  ChannelGradientVectorPostprocessor is a VectorPostprocessor that calculates
 *  the difference between two LineValueSampler vector postprocessors.
 */

class ChannelGradientVectorPostprocessor : public GeneralVectorPostprocessor
{
public:
  /**
    * Class constructor
    * @param parameters The input parameters
    */
  ChannelGradientVectorPostprocessor(const InputParameters & parameters);

  /**
   * Initialize, clears old results
   */
  virtual void initialize() override;

  /**
   * Perform the difference operation
   */
  virtual void execute() override;

protected:
  VectorPostprocessorName _lv1_name;
  VectorPostprocessorName _lv2_name;

  std::string _axis;

  /// The variable values from the line value samplers
  const VectorPostprocessorValue & _lv1_variable_values;
  const VectorPostprocessorValue & _lv2_variable_values;

  /// The axis coordinate values for the line value samplers
  const VectorPostprocessorValue & _lv1_axis_values;

  /// Axis coordinate values; corresponds to values from _lv1_axis_values
  VectorPostprocessorValue * _axis_values;

  /// The vector holding the difference between the two line value samplers
  VectorPostprocessorValue * _gradient_values;
};

#endif
