/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MULTIAPPMESHFUNCTIONTRANSFER_H
#define MULTIAPPMESHFUNCTIONTRANSFER_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppMeshFunctionTransfer;

template <>
InputParameters validParams<MultiAppMeshFunctionTransfer>();

/**
 * Transfers a vector of variables. For each individual one,
 * samples the variable's value in the Master domain at the point where
 * the MultiApp is. Copies that value into a postprocessor in the MultiApp.
 * The source and destination vectors (of variables) should be ordered consistently.
 */
class MultiAppMeshFunctionTransfer : public MultiAppTransfer
{
public:
  MultiAppMeshFunctionTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void execute() override;

protected:
  /// The vector of variables to transfer to
  std::vector<AuxVariableName> _to_var_name;
  /// The vector of variables to transfer from
  std::vector<VariableName> _from_var_name;
  /// The number of variables to transfer
  unsigned int _var_size;
  bool _error_on_miss;

private:
  /**
   * Performs the transfer for the variable of index i
   */
  void transferVariable(unsigned int i);

  /// To send points to other processors
  std::vector<std::vector<Parallel::Request>> _send_points;
  /// To send values to other processors
  std::vector<std::vector<Parallel::Request>> _send_evals;
  /// To send app ids to other processors
  std::vector<std::vector<Parallel::Request>> _send_ids;
};

#endif /* MULTIAPPMESHFUNCTIONTRANSFER_H */
