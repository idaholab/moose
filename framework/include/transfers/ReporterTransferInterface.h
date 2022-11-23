//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReporterData.h"
#include "FEProblemBase.h"

// Forward declarations
class Transfer;

/*
 * This transfer serves as a base class for transferring reporter values between
 * multiapps. This includes reporters, vector postprocessors, and postprocessors.
 *
 * The underlying purpose fo this class is to avoid using non-const access to ReporterData
 * through FEProblemBase to set reporter data. Instead, we simply have this class
 * as a friend to ReporterData::WriteKey and provide these protected functions for derived
 * classes. This avoids any old object modifying reporter data.
 */
class ReporterTransferInterface
{
public:
  static InputParameters validParams();
  ReporterTransferInterface(const Transfer * transfer);

protected:
  static MultiMooseEnum standardTransferTypes()
  {
    return MultiMooseEnum("bool=0 integer=1 real=2 string=3");
  }

  /*
   * This function allows derived objects to decide how the "from" reporters should be transferred.
   * I.e. whether we are transferring the entire data or part of it. Without calling this
   * early (constructor is preferred) there could be unintended behaiviour for
   * non-broadcasted data like VPPs.
   *
   * @param ReporterType data type of the reporter
   * @param name reporter name
   * @param problem The FEProblem that references the reporter data
   */
  void addReporterTransferMode(const ReporterName & name,
                               const ReporterMode & mode,
                               FEProblemBase & problem);

  /*
   * Transferring reporter value between FEProblems, mainly used for multiapps
   *
   * @param from_reporter reporter name on main app
   * @param to_reporter reporter name on sub app
   * @param from_problem The FEProblem that references the reporter data with the value
   * @param to_problem The FEProblem that references the reporter data to transfer to
   * @param time_index time index of transfer (default is lastest data)
   */
  void transferReporter(const ReporterName & from_reporter,
                        const ReporterName & to_reporter,
                        const FEProblemBase & from_problem,
                        FEProblemBase & to_problem,
                        unsigned int time_index = 0);

  /*
   * Transferring reporter value from one FEProblems to a vector reporter value from another,
   * mainly used for multiapps.
   *
   * @param from_reporter reporter name on main app
   * @param to_reporter reporter name on sub app
   * @param from_problem The FEProblem that references the reporter data with the value
   * @param to_problem The FEProblem that references the reporter data to transfer to
   * @param index the element index of the vector reporter
   * @param time_index time index of transfer (default is lastest data)
   */
  void transferToVectorReporter(const ReporterName & from_reporter,
                                const ReporterName & to_reporter,
                                const FEProblemBase & from_problem,
                                FEProblemBase & to_problem,
                                dof_id_type index,
                                unsigned int time_index = 0);

  /*
   * Helper for declaring a new reporter value in a FEProblem that is the same type
   * as the reporter value in another FEProblem.
   *
   * @param from_reporter reporter name to clone type
   * @param to_reporter reporter name of the clone
   * @param from_problem The FEProblem that references the reporter data with the value
   * @param to_problem The FEProblem that references the reporter data to declare clone
   * @param mode ReporterMode to declare value as
   */
  void declareClone(const ReporterName & from_reporter,
                    const ReporterName & to_reporter,
                    const FEProblemBase & from_problem,
                    FEProblemBase & to_problem,
                    const ReporterMode & mode);

  /*
   * Helper for declaring a new reporter value in a FEProblem that has specified type.
   *
   * @param reporter_name reporter name of the clone
   * @param fe_problem The FEProblem that references the reporter data to declare clone
   * @param type The type of reporter to declare
   * @param mode ReporterMode to declare value as
   */
  void declareClone(const ReporterName & rname,
                    FEProblemBase & problem,
                    const std::string & type,
                    const ReporterMode & mode);

  /*
   * Helper for declaring a new vector reporter value in a FEProblem that contains
   * the same type as the reporter value in another FEProblem.
   *
   * @param from_reporter reporter name to clone type
   * @param to_reporter reporter name of the vector clone
   * @param from_problem The FEProblem that references the reporter data with the value
   * @param to_problem The FEProblem that references the reporter data to declare value
   * @param mode ReporterMode to declare value as
   */
  void declareVectorClone(const ReporterName & from_reporter,
                          const ReporterName & to_reporter,
                          const FEProblemBase & from_problem,
                          FEProblemBase & to_problem,
                          const ReporterMode & mode);

  /*
   * Helper for declaring a new reporter value in a FEProblem that has specified type.
   *
   * @param reporter_name reporter name of the vector clone
   * @param fe_problem The FEProblem that references the reporter data to declare vector clone
   * @param type The type of reporter to declare the vector
   * @param mode ReporterMode to declare value as
   */
  void declareVectorClone(const ReporterName & rname,
                          FEProblemBase & problem,
                          const std::string & type,
                          const ReporterMode & mode);

  /*
   * Resize vector reporter value
   *
   * @param name Name of reporter
   * @param problem FEProblem that contains the reporter value
   * @param n New size of vector
   */
  void resizeReporter(const ReporterName & name, FEProblemBase & problem, dof_id_type n);

  /*
   * Helper for declaring reporter names when transfer is cloning values.
   * The result names will be:
   *      names[i] = obj_name/prefix:rep_name[i].getObjectName():rep_name[i].getValueName()
   *
   * @param prefix A string to prefix the reporter value name with.
   *               Typically the transfer name or user supplied.
   * @param obj_name The reporter object name that emulates holding the data
   * @param rep_names The list of reporter names that are being cloned
   *
   * @return A list of declarable reporter names
   */
  std::vector<ReporterName> getReporterNamesHelper(std::string prefix,
                                                   const std::string & obj_name,
                                                   const std::vector<ReporterName> & rep_names);

  /**
   * Checks if the problem \p problem has a Reporter value with the name \p reporter.
   */
  void checkHasReporterValue(const ReporterName & reporter, const FEProblemBase & problem) const;

private:
  /**
   * Helper for hiding the variables in the problem \p problem if the Reporter with name
   * \p reporter is associated with a Reporter object (not a PP or VPP)
   */
  void hideVariableHelper(const ReporterName & reporter, FEProblemBase & problem);

  /// The Transfer that this interface is associated with
  const Transfer & _rti_transfer;
};
