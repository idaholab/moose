#ifndef CHANNELGRADIENTVECTORPOSTPROCESSOR_H
#define CHANNELGRADIENTVECTORPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class ChannelGradientVectorPostprocessor;

template <>
InputParameters validParams<ChannelGradientVectorPostprocessor>();

/**
 *  ChannelGradientVectorPostprocessor is a VectorPostprocessor that performs a least squares
 *  fit on data calculated in another VectorPostprocessor.
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
   * Perform the least squares fit
   */
  virtual void execute() override;

protected:
  VectorPostprocessorName _lv1_name;
  VectorPostprocessorName _lv2_name;

  std::string _axis;

  /// The variables with the x, y data to be fit
  const VectorPostprocessorValue & _lv1_variable_values;
  const VectorPostprocessorValue & _lv2_variable_values;
  const VectorPostprocessorValue & _lv1_axis_values;

  VectorPostprocessorValue * _axis_values;
  VectorPostprocessorValue * _gradient_values;
};

#endif
