//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiAppTransfer.h"

#include "ReporterData.h"

/*
 * This transfer serves as a base class for transferring reporter values between
 * multiapps. This includes reporters, vector postprocessors, and postprocessors.
 *
 * The undelying purpose fo this class is to avoid using non-const access to ReporterData
 * through FEProblemBase to set reporter data. Instead, we simply have this class
 * as a friend to ReporterData and provide these protected functions for derived
 * classes. This avoids any ol' object modifying reporter data.
 */
class MultiAppReporterTransferBase : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppReporterTransferBase(const InputParameters & parameters);

protected:
  /*
   * Transferring reporter value to a multiapp
   *
   * @param from_reporter reporter name on main app
   * @param to_reporter reporter name on sub app
   * @param subapp_index the index of the sub app to transfer to
   * @param time_index time index of transfer (default is lastest data)
   */
  template <typename ReporterType>
  void transferToMultiApp(const ReporterName & from_reporter,
                          const ReporterName & to_reporter,
                          unsigned int subapp_index,
                          unsigned int time_index = 0);

  /*
   * Transferring reporter value from a multiapp
   *
   * @param from_reporter reporter name on sub app
   * @param to_reporter reporter name on main app
   * @param subapp_index the index of the sub app to transfer from
   * @param time_index time index of transfer (default is lastest data)
   */
  template <typename ReporterType>
  void transferFromMultiApp(const ReporterName & from_reporter,
                            const ReporterName & to_reporter,
                            unsigned int subapp_index,
                            unsigned int time_index = 0);
};

template <typename ReporterType>
void
MultiAppReporterTransferBase::transferToMultiApp(const ReporterName & from_reporter,
                                                 const ReporterName & to_reporter,
                                                 unsigned int subapp_index,
                                                 unsigned int time_index)
{
  const ReporterType & value =
      _multi_app->problemBase().getReporterData().template getReporterValue<ReporterType>(
          from_reporter, time_index);

  _multi_app->appProblemBase(subapp_index)
      .getReporterData(ReporterData::WriteKey())
      .template setReporterValue<ReporterType>(to_reporter, value, time_index);
}

template <typename ReporterType>
void
MultiAppReporterTransferBase::transferFromMultiApp(const ReporterName & from_reporter,
                                                   const ReporterName & to_reporter,
                                                   unsigned int subapp_index,
                                                   unsigned int time_index)
{
  const ReporterType & value =
      _multi_app->appProblemBase(subapp_index)
          .getReporterData()
          .template getReporterValue<ReporterType>(from_reporter, time_index);

  _multi_app->problemBase()
      .getReporterData(ReporterData::WriteKey())
      .template setReporterValue<ReporterType>(to_reporter, value, time_index);
}
