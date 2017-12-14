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

#ifndef MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H
#define MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H

// MOOSE includes
#include "MultiAppTransfer.h"

// Module includes
#include "MutableCoefficientsInterface.h"

// Forward declations
class MultiAppMutableCoefficientsTransfer;

template <>
InputParameters validParams<MultiAppMutableCoefficientsTransfer>();

/// Transfers mutable coefficient arrays between supported object types
class MultiAppMutableCoefficientsTransfer : public MultiAppTransfer
{
public:
  MultiAppMutableCoefficientsTransfer(const InputParameters & parameters);

  // Virtual overrides
  virtual void initialSetup() override;
  virtual void execute() override;

  /// user-defined pretty name for this transfer, defaults to 'MultiAppFETransfer'
  const std::string _pretty_name;

protected:
  /// Name of the MutableCoefficientsInterface-derived object in the creating app
  const std::string _this_app_object_name;
  /// Name of the MutableCoefficientsInterface-derived object in the MultiApp
  const std::string _multi_app_object_name;

private:
  /**
   * Function pointer typedef for functions used to find, convert, and return
   * the appropriate MutableCoefficientsInterface object from an FEProblemBase
   */
  typedef MutableCoefficientsInterface & (MultiAppMutableCoefficientsTransfer::*GetProblemObject)(
      FEProblemBase & base, const std::string & object_name, THREAD_ID thread);
  /**
   * Searches an FEProblemBase for a MutableCoefficientsInterface-based object
   * and returns a function pointer to the matched function type
   */
  virtual GetProblemObject scanProblemBaseForObject(FEProblemBase & base,
                                                    const std::string & object_name,
                                                    const std::string & app_name);
  /// Gets a MutableCoefficientsInterface-based Function
  MutableCoefficientsInterface & getMutableCoefficientsFunction(FEProblemBase & base,
                                                                const std::string & object_name,
                                                                THREAD_ID thread);
  /// Gets a MutableCoefficientsInterface-based UserObject
  MutableCoefficientsInterface & getMutableCoefficientsUserOject(FEProblemBase & base,
                                                                 const std::string & object_name,
                                                                 THREAD_ID thread);

  /// Function pointer for grabbing the MultiApp object
  GetProblemObject getMultiAppObject;
  /// Function pointer for grabbing the SubApp object
  GetProblemObject getSubAppObject;
};

// Typedef for the functional_expansion module
typedef MultiAppMutableCoefficientsTransfer MultiAppFETransfer;

#endif // MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H
