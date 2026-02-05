//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimesInterface.h"
#include "TimesReporter.h"
#include "FEProblemBase.h"

InputParameters
TimesInterface::validParams()
{
  return emptyInputParameters();
}

TimesInterface::TimesInterface(const MooseObject * moose_object)
  : _tmi_moose_object(*moose_object),
    _tmi_params(moose_object->parameters()),
    _tmi_feproblem(*_tmi_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tmi_tid(_tmi_params.have_parameter<THREAD_ID>("_tid") ? _tmi_params.get<THREAD_ID>("_tid") : 0)
{
}

const Times &
TimesInterface::getTimes(const std::string & param, bool allow_unsorted) const
{
  _curr_param = param;
  const auto & times = getTimesByName(_tmi_params.get<TimesName>(param), allow_unsorted);
  _curr_param.clear();
  return times;
}

const Times *
TimesInterface::getOptionalTimes(const std::string & param, bool allow_unsorted) const
{
  _curr_param = param;
  const auto * times = _tmi_params.isParamValid(param) ? &getTimes(param, allow_unsorted) : nullptr;
  _curr_param.clear();
  return times;
}

const Times &
TimesInterface::getTimesByName(const TimesName & name, bool allow_unsorted) const
{
  const Times * times_obj = nullptr; // Returned value

  // See if the passed in name is a user object
  if (_tmi_feproblem.hasUserObject(name))
  {
    // If this is a user object, we want to make sure we add proper dependencies
    auto uoi = dynamic_cast<const UserObjectInterface *>(&_tmi_moose_object);

    // Pointer to TimesReporter, used to throw useful error if name is right, but wrong object
    auto tr = dynamic_cast<const TimesReporter *>(
        uoi ? &uoi->getUserObjectBaseByName(name)
            : &_tmi_feproblem.getUserObjectBase(name, _tmi_tid));
    if (!tr)
      timesError(name, " is not a Times object.");

    times_obj = tr;
  }

  // Attempt to convert the point to a vector of reals to make a default Times object
  else if (getDefaultTimesObject(name, allow_unsorted))
    times_obj = &_default_times_objs.back();

  // Cannot do anything from here.
  else
    timesError("Could not find times object ", name, ".");

  // TODO: Add ability to retrieve the object later in case this operation
  // happens before the Times objects are constructed

  return *times_obj;
}

bool
TimesInterface::getDefaultTimesObject(const std::string & input, bool allow_unsorted) const
{
  std::vector<Real> times;
  if (MooseUtils::tokenizeAndConvert(input, times))
  {
    if (!allow_unsorted && !std::is_sorted(times.begin(), times.end()))
      timesError("Times are not sorted:\n    ", Moose::stringify(times, ", ", "", true));
    _default_times_objs.emplace_back(times, _tmi_feproblem);
    return true;
  }
  else
    return false;
}

TimesInterface::DefaultTimes::DefaultTimes(const std::vector<Real> & times,
                                           const FEProblemBase & fe_problem)
  : Times(_default_times, fe_problem.time(), /*is dynamic*/ false), _default_times(times)
{
}
