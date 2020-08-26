//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "MooseEnum.h"

/**
 * Reporter producer/consumer modes
 *
 * @see Reporter, ReporterState, ReporterContext
 *
 * This is used to define the "mode" in which the value is being produced or consumed. The default
 * for both the producer and consumer is ROOT. This means that it is assumed that the
 * correct/complete value of the Reporter value exists on the root processor.
 *
 * On the producer side, it is the responsibility of the Reporter object to perform the necessary
 * operations to compute the value in the specified mode. The ReporterContext objects are helpers
 * for the producer to get the value in the desired mode.
 *
 * On the consumer side, if needed the data supplied will be converted to the correct mode
 * automatically if needed and  possible. If the producer mode is not compatibable with
 * the consumer mode, then an error is produced. The base ReporterContext handles the automatic
 * operations and error handling.
 *
 * The UNSET is used as the default to allow the ReporterContext objects to set the value
 * programmatically and error if it is set to something wrong by the user.
 */
class ReporterMode;
extern const ReporterMode REPORTER_MODE_UNSET;
extern const ReporterMode REPORTER_MODE_ROOT;
extern const ReporterMode REPORTER_MODE_REPLICATED;
extern const ReporterMode REPORTER_MODE_DISTRIBUTED;

/**
 * MooseEnumItem that automatically creates the ID and doesn't allow the ID to be assigned
 *
 * This protects user from hitting an ID collision when creating custom modes.
 */
class ReporterMode : public MooseEnumItem
{
public:
  ReporterMode(const std::string & key);

private:
  // Automatically incremented in construction
  static int ID_COUNTER;
};

/**
 * MooseEnum designed for the ReporterContext objects to define how a ReporterValue can and is
 * being produced. The available items indicate how it can be produced and the current item is
 * how the value is being produced.
 *
 * @see ReporterContext.h
 */
class ReporterProducerEnum : public MooseEnum
{
public:
  ReporterProducerEnum();

  /// Clear all available items (i.e., create an empty MooseEnum)
  void clear();

  ///@{
  /// Add possible items to the enumeration.
  template <typename... Args>
  void insert(const ReporterMode & mode, Args... modes);
  void insert(const ReporterMode & mode);
  ///@}
};

template <typename... Args>
void
ReporterProducerEnum::insert(const ReporterMode & mode, Args... modes)
{
  insert(mode);
  insert(modes...);
}
