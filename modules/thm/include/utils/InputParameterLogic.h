#ifndef INPUTPARAMETERLOGIC_H
#define INPUTPARAMETERLOGIC_H

#include "MooseObject.h"

/**
 * Determines values of parameters for which there are one-phase and two-phase variants.
 *
 * This function aims to achieve the following behavior:
 * - If a user provides a two-phase parameter for one-phase flow, give warning.
 * - If a user does not provide a two-phase parameter for two-phase flow but
 *   provides the one-phase parameter, use the one-phase value for the two-phase
 *   parameter.
 *
 * @warning If the parameters do not all have defaults, it is recommended to
 * perform the steps in the following order:
 *   -# call this function
 *   -# call \c getParam(), guarded with \c isParamValid()
 *   .
 * rather than the following approach:
 *   -# create a pointer to the parameter, guarded with \c isParamValid()
 *   -# call this function
 *   -# call \c getParam(), guarded with \c isParamValid()
 *   .
 * In the latter approach, one may cause a segfault because the pointer was
 * originally set to \c nullptr because of the \c isParamValid() guard, which can
 * change with a call to this function.
 *
 * @param[in] is_two_phase  flag denoting that the object is two-phase
 * @param[in] one_phase_param  name of one-phase parameter
 * @param[in] two_phase_params  vector of names of two-phase parameters
 * @param[in,out] object  object for which parameters may be altered
 */
template <typename T>
void
getOneOrTwoPhaseParameters(const bool & is_two_phase,
                           const std::string & one_phase_param,
                           const std::vector<std::string> & two_phase_params,
                           MooseObject & object)
{
  InputParameters & params = const_cast<InputParameters &>(object.parameters());

  if (is_two_phase)
  {
    // get the one-phase parameter in case it is used as two-phase default
    // this must be guarded with isParamValid() because a default may not be provided
    const T * const one_phase_value =
        params.isParamValid(one_phase_param) ? &object.getParam<T>(one_phase_param) : nullptr;

    // if one-phase value is provided, use it as default for all two-phase values
    // otherwise keep the two-phase parameter defaults
    for (unsigned int i = 0; i < two_phase_params.size(); ++i)
      if (!(params.isParamSetByUser(two_phase_params[i])))
        if (params.isParamSetByUser(one_phase_param))
        {
          // guard null pointer, though it should always be true that if isParamSetByUser() is true,
          // then isParamValid() is true
          if (!params.isParamValid(one_phase_param))
            mooseError(object.name(),
                       ": isParamSetByUser() returns TRUE, but ",
                       "isParamValid() returns FALSE for parameter '",
                       one_phase_param,
                       "'.");

          params.set<T>(two_phase_params[i]) = *one_phase_value;
        }
  }
  else
  {
    // provide warning if any two-phase parameters were provided for one-phase flow
    for (unsigned int i = 0; i < two_phase_params.size(); ++i)
      if (params.isParamSetByUser(two_phase_params[i]))
        mooseWarning(object.name(),
                     ": Provided two-phase parameter for one-phase flow: '",
                     two_phase_params[i],
                     "'. This value will not be used.");
  }
}

#endif // INPUTPARAMETERLOGIC_H
