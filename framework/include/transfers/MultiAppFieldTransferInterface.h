//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPFIELDTRANSFERINTERFACE_H
#define MULTIAPPFIELDTRANSFERINTERFACE_H

#include "MultiAppTransfer.h"

// Forward declarations
class MultiAppFieldTransferInterface;

template <>
InputParameters validParams<MultiAppFieldTransferInterface>();

/**
 *  This serves an interface for MultiAppInterpolationTransfer, MultiAppNearestNodeTransfer and so
 * on.
 */
class MultiAppFieldTransferInterface : public MultiAppTransfer
{
public:
  MultiAppFieldTransferInterface(const InputParameters & parameters);

  /**
   * Add some extra work if necessary after execute(). For example, adjust the solution
   * to preserve some physics quality of interest.
   */
  virtual void postExecute();

protected:
  std::vector<VariableName> _from_var_name;
  std::vector<AuxVariableName> _to_var_name;

  // If this transfer is going to conserve the physics
  bool _preserve_transfer;
  // Postprocessor evaluates an adjuster for the source physics
  std::vector<PostprocessorName> _from_postprocessor_to_be_preserved;
  // Postprocessor evaluates an adjuster for the target physics
  std::vector<PostprocessorName> _to_postprocessor_to_be_preserved;

private:
  void adjustTransferedSolution(FEProblemBase * from_problem,
                                PostprocessorName & from_postprocessor,
                                FEProblemBase & to_problem,
                                PostprocessorName & to_postprocessor);
};

#endif // MULTIAPPFIELDTRANSFERINTERFACE_H
