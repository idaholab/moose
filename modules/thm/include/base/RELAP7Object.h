#ifndef RELAP7OBJECT_H
#define RELAP7OBJECT_H

#include "MooseObject.h"

class RELAP7Object;

template <>
InputParameters validParams<RELAP7Object>();

/**
 *
 */
class RELAP7Object : public MooseObject
{
public:
  RELAP7Object(const InputParameters & parameters);

protected:
  /**
   * Passes a parameter from this object's input parameters to another set of input parameters.
   *
   * @tparam T  the type of the parameter to be passed
   * @param name[in] name  the name of this object's parameter
   * @param new_name[in] new_name  the name of the corresponding parameters in \c params
   * @param params[in,out] params  the parameters to which the parameter will be passed
   */
  template <typename T>
  void passParameter(const std::string & name,
                     const std::string & new_name,
                     InputParameters & params) const;

  /**
   * Passes a parameter from this object's input parameters to another set of input parameters.
   *
   * This version overloads the other by assuming that the parameter has the same name.
   *
   * @tparam T  the type of the parameter to be passed
   * @param name[in] name  the name of the parameter
   * @param params[in,out] params  the parameters to which the parameter will be passed
   */
  template <typename T>
  void passParameter(const std::string & name, InputParameters & params) const;
};

template <typename T>
void
RELAP7Object::passParameter(const std::string & name,
                            const std::string & new_name,
                            InputParameters & params) const
{
  if (isParamValid(name))
    params.set<T>(new_name) = _pars.get<T>(name);
}

template <typename T>
void
RELAP7Object::passParameter(const std::string & name, InputParameters & params) const
{
  passParameter<T>(name, name, params);
}

#endif /* RELAP7OBJECT_H */
