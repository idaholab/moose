/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERTRANSFER_H
#define SAMPLERTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"
#include "Sampler.h"

// Forward declarations
class SamplerTransfer;
class SamplerReceiver;

template <>
InputParameters validParams<SamplerTransfer>();

/**
 * Copy each row from each DenseMatrix to the sub-applications SamplerReceiver object.
 */
class SamplerTransfer : public MultiAppTransfer
{
public:
  SamplerTransfer(const InputParameters & parameters);
  virtual void execute() override;

protected:
  /**
   * Return the SamplerReceiver object and perform error checking.
   * @param app_index The global sup-app index
   */
  SamplerReceiver * getReceiver(unsigned int app_index,
                                const std::vector<DenseMatrix<Real>> & samples);

  /// Storage for the list of parameters to control
  const std::vector<std::string> & _parameter_names;

  /// Pointer to the Sampler object used by the SamplerMultiApp
  Sampler * _sampler_ptr;

  /// The name of the SamplerReceiver Control object on the sub-application
  const std::string & _receiver_name;

  /// The matrix and row for each MultiApp
  std::vector<std::pair<unsigned int, unsigned int>> _multi_app_matrix_row;
};

#endif
