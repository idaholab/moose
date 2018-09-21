//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEVARIABLESCALAR_H
#define MOOSEVARIABLESCALAR_H

#include "MooseVariableBase.h"

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class Assembly;

/**
 * Class for scalar variables (they are different).
 */
class MooseVariableScalar : public MooseVariableBase
{
public:
  MooseVariableScalar(unsigned int var_num,
                      const FEType & fe_type,
                      SystemBase & sys,
                      Assembly & assembly,
                      Moose::VarKindType var_kind,
                      THREAD_ID tid);
  virtual ~MooseVariableScalar();

  void reinit();

  virtual bool isNodal() const override;
  virtual bool isVector() const override;

  //
  VariableValue & sln() { return _u; }
  VariableValue & slnOld() { return _u_old; }
  VariableValue & slnOlder() { return _u_older; }
  VariableValue & vectorTagSln(TagID tag)
  {
    _need_vector_tag_u[tag] = true;
    return _vector_tag_u[tag];
  }
  VariableValue & matrixTagSln(TagID tag)
  {
    _need_matrix_tag_u[tag] = true;
    return _matrix_tag_u[tag];
  }

  VariableValue & uDot()
  {
    _need_u_dot = true;
    return _u_dot;
  }

  VariableValue & uDotDot()
  {
    _need_u_dotdot = true;
    return _u_dotdot;
  }

  VariableValue & uDotOld()
  {
    _need_u_dot_old = true;
    return _u_dot_old;
  }

  VariableValue & uDotDotOld()
  {
    _need_u_dotdot_old = true;
    return _u_dotdot_old;
  }

  VariableValue & duDotDu()
  {
    _need_du_dot_du = true;
    return _du_dot_du;
  }

  VariableValue & duDotDotDu()
  {
    _need_du_dotdot_du = true;
    return _du_dotdot_du;
  }

  /**
   * Set the nodal value for this variable (to keep everything up to date
   */
  void setValue(unsigned int i, Number value);

  /**
   * Set all of the values of this scalar variable to the same value
   */
  void setValues(Number value);

  void insert(NumericVector<Number> & soln);

protected:
  /// The assembly
  Assembly & _assembly;

  /// The value of scalar variable
  VariableValue _u;
  /// The old value of scalar variable
  VariableValue _u_old;
  /// The older value of scalar variable
  VariableValue _u_older;
  /// Tagged vectors
  std::vector<VariableValue> _vector_tag_u;
  /// Only cache data when need it
  std::vector<bool> _need_vector_tag_u;
  /// Tagged matrices
  std::vector<VariableValue> _matrix_tag_u;
  /// Only cache data when need it
  std::vector<bool> _need_matrix_tag_u;

  VariableValue _u_dot;
  VariableValue _u_dotdot;
  VariableValue _u_dot_old;
  VariableValue _u_dotdot_old;
  VariableValue _du_dot_du;
  VariableValue _du_dotdot_du;

  bool _need_u_dot;
  bool _need_u_dotdot;
  bool _need_u_dot_old;
  bool _need_u_dotdot_old;
  bool _need_du_dot_du;
  bool _need_du_dotdot_du;
};

#endif /* MOOSEVARIABLESCALAR_H */
