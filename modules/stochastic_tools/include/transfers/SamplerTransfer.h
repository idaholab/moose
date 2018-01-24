//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
