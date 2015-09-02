//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// MetaPhysicL - A metaprogramming library for physics calculations
//
// Copyright (C) 2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor,
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
//
// $Id: core.h 37197 2013-02-21 05:49:09Z roystgnr $
//
//--------------------------------------------------------------------------

#ifndef METAPHYSICL_PHYSICS_H
#define METAPHYSICL_PHYSICS_H

#include "metaphysicl/ct_set.h"

#include <functional>
#include <ostream>
#include <type_traits>

namespace MetaPhysicL {

//using MetaPhysicL::IfElse;
//using MetaPhysicL::TypesEqual;
//using MetaPhysicL::Container;
//using MetaPhysicL::NullContainer;
//using MetaPhysicL::NullType;
//using MetaPhysicL::UnsignedIntType;
//using MetaPhysicL::UIntSetConstructor;
//using MetaPhysicL::SetConstructor;
//using MetaPhysicL::SetOfSetsUnion;

#define StateInput(type, name) \
    const State::entry_type<type::value>::type& name = state.template get<type>()

#define StateOutput(type, name) \
    State::entry_type<type::value>::type& name = state.template get<type>()

enum VarIndexType {
  DENSITIES_VAR,
  DENSITY_VAR,
  MOMENTUM_VAR,
  VELOCITY_VAR,
  SPEED_SQUARED_VAR,
  ENERGY_VAR,
  SPECIFIC_ENERGY_VAR,
  SPECIFIC_INTERNAL_ENERGY_VAR,
  TEMPERATURE_VAR,
  TRANSLATIONAL_ROTATIONAL_SPECIFIC_HEAT_VAR,
  TRANSLATIONAL_ROTATIONAL_ENERGY_VAR
};


#define DENSITIES_VAR_NAME                               rhoi
#define DENSITY_VAR_NAME                                 rho
#define MOMENTUM_VAR_NAME                                rhoU
#define VELOCITY_VAR_NAME                                U
#define SPEED_SQUARED_VAR_NAME                           UdotU
#define ENERGY_VAR_NAME                                  rhoE
#define SPECIFIC_ENERGY_VAR_NAME                         E
#define SPECIFIC_INTERNAL_ENERGY_VAR_NAME                e
#define TEMPERATURE_VAR_NAME                             T
#define TRANSLATIONAL_ROTATIONAL_SPECIFIC_HEAT_VAR_NAME  cv_tr
#define TRANSLATIONAL_ROTATIONAL_ENERGY_VAR_NAME         e_tr

template <typename SubFunctor>
struct UnaryPhysics
{
  template <typename inputtype, typename outputtype>
  struct as_unary_function :
    public std::unary_function<inputtype, outputtype>
  {
    outputtype operator() (const inputtype& in)
    {
      return _f(in);
    }
  };

private:
  SubFunctor _f;
};

template <typename SubFunctor>
struct BinaryPhysics
{
  template <typename input1type, typename input2type, typename outputtype>
  struct as_binary_function :
    public std::binary_function<input1type, input2type, outputtype>
  {
    outputtype operator()
      (const input1type& in1, const input2type& in2) 
    {
      return _f(in1, in2);
    }
  };

private:
  SubFunctor _f;
};

#define DeclareUnaryPhysics(physicsname, input1enum, outputenum, code) \
struct physicsname \
{ \
  static const char* name() { return #physicsname; } \
  static const MetaPhysicL::VarIndexType input1index = input1enum; \
  static const MetaPhysicL::VarIndexType outputindex = outputenum; \
 \
  template <typename inputtype> \
  static auto evaluate (const inputtype& input1enum##_NAME) \
    -> decltype(code) \
  { \
    return code; \
  } \
 \
  template <typename inputtype> \
  inline auto operator() (const inputtype& input1enum##_NAME) \
    -> decltype(evaluate(input1enum##_NAME)) \
  { \
    return evaluate(input1enum##_NAME); \
  } \
 \
  template <typename inputstate, typename outputstate> \
  static void update_state (const inputstate& in, outputstate& out) \
  { \
    out.template var<outputenum>() = \
      evaluate(in.template var<input1enum>()); \
  } \
 \
  typedef UIntSetConstructor<input1enum>::type inputset; \
  typedef UIntSetConstructor<outputenum>::type outputset; \
 \
  template <typename inputset> \
  struct OutputSet { \
    typedef typename SetConstructor< \
      UnsignedIntType< \
        outputenum, \
        typename std::remove_const< \
          typename std::remove_reference< \
            decltype(physicsname::evaluate \
              (typename inputset::template ElementOf< \
                UnsignedIntType<input1enum> >::type::data_type())) \
            >::type \
          >::type \
        > \
    >::type type; \
  }; \
}

#define DeclareBinaryPhysics(physicsname, input1enum, input2enum, outputenum, code) \
struct physicsname \
{ \
  static const char* name() { return #physicsname; } \
  static const MetaPhysicL::VarIndexType input1index = input1enum; \
  static const MetaPhysicL::VarIndexType input2index = input2enum; \
  static const MetaPhysicL::VarIndexType outputindex = outputenum; \
 \
  template <typename input1type, typename input2type> \
  static auto evaluate (const input1type& input1enum##_NAME, \
                        const input2type& input2enum##_NAME) \
    -> decltype(code) \
  { \
    return code; \
  } \
 \
  template <typename input1type, typename input2type> \
  inline auto operator() (const input1type& input1enum##_NAME, \
                          const input2type& input2enum##_NAME) \
    -> decltype(evaluate(input1enum##_NAME, input2enum##_NAME)) \
  { \
    return evaluate(input1enum##_NAME, input2enum##_NAME); \
  } \
 \
  template <typename inputstate, typename outputstate> \
  static void update_state (const inputstate& in, outputstate& out) \
  { \
    out.template var<outputenum>() = \
      evaluate(in.template var<input1enum>(), \
               in.template var<input2enum>()); \
  } \
 \
  typedef UIntSetConstructor<input1enum,input2enum>::type inputset; \
  typedef UIntSetConstructor<outputenum>::type outputset; \
 \
  template <typename inputset> \
  struct OutputSet { \
    typedef typename SetConstructor< \
      UnsignedIntType< \
        outputenum, \
        typename std::remove_const< \
          typename std::remove_reference< \
            decltype(physicsname::evaluate \
              (typename inputset::template ElementOf< \
                 UnsignedIntType<input1enum> >::type::data_type(), \
               typename inputset::template ElementOf< \
                 UnsignedIntType<input2enum> >::type::data_type())) \
            >::type \
          >::type \
        > \
    >::type type; \
  }; \
}

#define DeclareTrinaryPhysics(physicsname, input1enum, input2enum, input3enum, outputenum, code) \
struct physicsname \
{ \
  static const char* name() { return #physicsname; } \
  static const MetaPhysicL::VarIndexType input1index = input1enum; \
  static const MetaPhysicL::VarIndexType input2index = input2enum; \
  static const MetaPhysicL::VarIndexType input3index = input3enum; \
  static const MetaPhysicL::VarIndexType outputindex = outputenum; \
 \
  template <typename input1type, typename input2type, typename input3type> \
  static auto evaluate (const input1type& input1enum##_NAME, \
                        const input2type& input2enum##_NAME, \
                        const input3type& input3enum##_NAME) \
    -> decltype(code) \
  { \
    return code; \
  } \
 \
  template <typename input1type, typename input2type, typename input3type> \
  inline auto operator() (const input1type& input1enum##_NAME, \
                          const input2type& input2enum##_NAME, \
                          const input3type& input3enum##_NAME) \
    -> decltype(evaluate(input1enum##_NAME, input2enum##_NAME, input3enum##_NAME)) \
  { \
    return evaluate(input1enum##_NAME, input2enum##_NAME, input3enum##_NAME); \
  } \
 \
  template <typename inputstate, typename outputstate> \
  static void update_state (const inputstate& in, outputstate& out) \
  { \
    out.template var<outputenum>() = \
      evaluate(in.template var<input1enum>(), \
               in.template var<input2enum>(), \
               in.template var<input3enum>()); \
  } \
 \
  typedef UIntSetConstructor<input1enum,input2enum,input3enum>::type inputset; \
  typedef UIntSetConstructor<outputenum>::type outputset; \
 \
  template <typename inputset> \
  struct OutputSet { \
    typedef typename SetConstructor< \
      UnsignedIntType< \
        outputenum, \
        typename std::remove_const< \
          typename std::remove_reference< \
            decltype(evaluate \
              (typename inputset::template ElementOf< \
                 UnsignedIntType<input1enum> >::type::data_type(), \
               typename inputset::template ElementOf< \
                 UnsignedIntType<input2enum> >::type::data_type(), \
               typename inputset::template ElementOf< \
                 UnsignedIntType<input3enum> >::type::data_type())) \
            >::type \
          >::type \
        > \
    >::type type; \
  }; \
}


template <typename StateType>
struct EvaluatePhysics {
  EvaluatePhysics(StateType& state) : _state(state) {}

  template <typename ValueType>
  void operator()() const {
    ValueType::update_state(_state, _state);
  };

  StateType& _state;
};


// A DependencyInsert takes a dependency set (set-of-Uint<T,set-of-Uint>s,
// where the first integer is a target variable index and the associated set is the
// variables whose values affect it) and returns a type which adds the
// dependencies from new_dependency (a Uint<T,set-of-Uints>), resolving
// dependencies-of-dependencies along the way.
template <typename prev_dependencies,
          typename new_dependency>
struct DependencyInsert
{
  typedef typename prev_dependencies::template
    Intersection<typename new_dependency::data_type>::type recursing_dependencies;

  typedef typename new_dependency::template rebind<
    typename new_dependency::data_type::template Union<
      typename SetOfSetsUnion<recursing_dependencies>::type
    >::type
  >::other complete_new_dependency;

  typedef Container<
    typename IfElse<
      (prev_dependencies::head_type::data_type::template
        Contains<new_dependency>::value),
      typename prev_dependencies::head_type::template rebind<
        typename prev_dependencies::head_type::data_type::template
          Union<typename complete_new_dependency::data_type>::type
      >::other,
      typename prev_dependencies::head_type
    >::type,
    typename DependencyInsert<typename prev_dependencies::tail_set,
                              complete_new_dependency>::type
  > type;
};

template <typename NullHeadType, typename new_dependency>
struct DependencyInsert<NullContainer<NullHeadType>, new_dependency>
{
  typedef Container<new_dependency, NullContainer<NullHeadType> > type;
};


// The DependencyUnion takes two dependency sets (set-of-Uint<T,set-of-Uint>s,
// where the first integer is a target variable index and the associated set is the
// variables whose values affect it) and returns a type which joins the two.  E.g.
// if a depends on b in prev_dependencies, and b depends on c in new_dependencies, then
// a depends on b and c in DependencyUnion<prev_dependencies,new_dependencies>::type
template <typename prev_dependencies,
          typename new_dependencies>
struct DependencyUnion
{
  typedef typename DependencyUnion<
    typename DependencyInsert<prev_dependencies, typename new_dependencies::head_type>::type,
    typename new_dependencies::tail_set
  >::type type;
};


template <typename prev_dependencies, typename NullHeadType>
struct DependencyUnion<prev_dependencies, NullContainer<NullHeadType> >
{
  typedef prev_dependencies type;
};


// Given a physics equation, EquationDependencies<equation>::type is
// the set-of-sets describing that equation's variable dependencies
template <typename equation>
struct EquationDependencies
{
  typedef typename equation::outputset::template rebind<
    typename equation::inputset
  >::other type;
};


template <>
struct EquationDependencies<NullType>
{
  typedef NullContainer<UnsignedIntType<0> > type;
};


// We use a forwarding struct here so we can specialize on NullType
template <typename equation>
struct GetOutputSet
{
  typedef typename equation::outputset type;
};


template <>
struct GetOutputSet<NullType>
{
  typedef NullContainer<UnsignedIntType<0> > type;
};


// The Equations struct takes a Container of physics functors,
// and typedefs various types defined by those functors
template <typename EquationList>
struct Equations
{
  typedef EquationList equation_list;

  typedef typename EquationList::head_type first_equation;
  typedef typename EquationList::tail_set  tail_equations;

  // outputset is the set of all variables that are the output of any
  // functor
  typedef typename
    first_equation::outputset::template Union<
      typename Equations<tail_equations>::outputset
    >::type outputset;

  // inputset is the set of all variables that are an input to any
  // functor
  typedef typename
    first_equation::inputset::template Union<
      typename Equations<tail_equations>::inputset
    >::type inputset;

  // inputset is the set of all variables that are an input or output
  // of any functor
  typedef typename inputset::template Union<outputset>::type
    varset;

  // FirstForwardSolvable<set_solved>::type, given the already-solved variables
  // set_solved, returns the type of the next functor in EquationList that can
  // be forward solved (i.e. directly evaluated)
  template <typename set_solved>
  struct FirstForwardSolvable
  {
    typedef typename IfElse<
      (is_null_container<
         typename first_equation::inputset::template Difference<set_solved>::type
       >::value &&
       is_null_container<
         typename first_equation::outputset::template Intersection<set_solved>::type
       >::value),
      first_equation,
      typename Equations<tail_equations>::template FirstForwardSolvable<set_solved>::next_to_solve
    >::type next_to_solve;

    typedef typename set_solved::template
      Union<typename GetOutputSet<next_to_solve>::type>::type current_solved;

    typedef typename IfElse<
      (TypesEqual<next_to_solve, NullType>::value),
      NullContainer<typename set_solved::head_type>,
      typename equation_list::template Difference<
        Container<next_to_solve, 
                  NullContainer<typename set_solved::head_type>,
                  typename equation_list::comparison>
      >::type
    >::type
      remaining_equations;
  };

  template <typename prev_solved,
            typename prev_dependencies,
            typename vars_to_consider=varset>
  struct DependencyFinder
  {
    typedef typename DependencyUnion<
      prev_dependencies,
      typename EquationDependencies<
        typename FirstForwardSolvable<prev_solved>::next_to_solve
      >::type
    >::type current_dependencies;

    typedef typename Equations<
      typename FirstForwardSolvable<prev_solved>::remaining_equations
    >::template DependencyFinder<
      typename FirstForwardSolvable<prev_solved>::current_solved,
      current_dependencies, vars_to_consider
    >::type type;
  };

  // The SolveList struct generates a list of equations to be
  // evaluated to produce the variables in set_to_solve, from the
  // inputs in set_solved, while also computing any unsolved intermediate
  // variables in set_intermediate.  Users may want to use SolveList instead,
  // which generates set_intermediate itself.
  template <typename set_solved,
            typename set_to_solve,
            typename set_intermediate>
  struct StrictSolveList
  {
    typedef typename Equations<
      typename FirstForwardSolvable<set_solved>::remaining_equations
    >::template StrictSolveList<
      typename FirstForwardSolvable<set_solved>::current_solved,
      set_to_solve, set_intermediate 
    >::type solve_list_tail;

    typedef typename IfElse<
      (TypesEqual<typename FirstForwardSolvable<set_solved>::next_to_solve,
                  NullType>::value),
      solve_list_tail,
      Container<
        typename FirstForwardSolvable<set_solved>::next_to_solve,
        solve_list_tail,
        typename equation_list::comparison
      >
    >::type type;
  };

  // The SolveList struct generates a list of equations to be
  // evaluated to produce the variables in set_to_solve, from the
  // inputs in set_solved, by potentially using any of the
  // intermediate values in vars_to_consider
  template <typename set_solved,
            typename set_to_solve>
  struct SolveList
  {
    typedef typename DependencyFinder<
      set_solved, NullContainer<typename set_solved::head_type>, varset
    >::type
      var_dependency_set_of_sets;

    typedef typename var_dependency_set_of_sets::template Union<set_to_solve>::type
      vars_involved;

    typedef typename StrictSolveList<set_solved, set_to_solve, vars_involved>::type type;
  };

  // The State is a data structure defined to hold the specified
  // input values and types in set_solved_with_types, the properly
  // typed output values in set_to_solve, and any intermediate values.
  template <typename set_solved_with_types,
            typename solve_list>
  struct SolveState
  {
    typedef typename solve_list::head_type::
      template OutputSet<set_solved_with_types>::type
      next_output_set;

    typedef typename 
      set_solved_with_types::template Union<next_output_set>::type
      next_solved_set;

    typedef typename
      SolveState<next_solved_set, typename solve_list::tail_set>::type
      type;
  };

  template <typename set_solved_with_types, typename NullHeadType>
  struct SolveState<set_solved_with_types,NullContainer<NullHeadType> >
  {
    typedef set_solved_with_types type;
  };
};

template <typename NullHeadType>
struct Equations<NullContainer<NullHeadType> >
{
  typedef NullContainer<NullHeadType> outputset;
  typedef NullContainer<NullHeadType> inputset;
  typedef NullContainer<NullHeadType> varset;

  template <typename set_solved>
  struct FirstForwardSolvable {
    typedef NullType next_to_solve;
    typedef set_solved current_solved;
    typedef NullContainer<NullHeadType> remaining_equations;
  };

  template <typename prev_solved,
            typename prev_dependencies,
            typename vars_to_consider=varset>
  struct DependencyFinder
  {
    typedef prev_dependencies type;
  };

  template <typename set_solved,
            typename set_to_solve,
            typename set_intermediate>
  struct StrictSolveList
  {
    typedef NullContainer<NullHeadType> type;
  };

  template <typename set_solved,
            typename set_to_solve>
  struct SolveList
  {
    typedef NullContainer<NullHeadType> var_dependency_set_of_sets;

    typedef set_to_solve vars_involved;

    typedef NullContainer<NullHeadType> type;
  };
};

struct NamedContainerOutputFunctor {
  NamedContainerOutputFunctor(std::ostream& o) : _out(o) {}

  template <typename ValueType>
  inline void operator()() const {
    _out << ", " << ValueType::name();
  }
private:
  std::ostream& _out;
};


template <typename C>
struct NamedContainer
{
};


template <typename C>
inline
std::ostream&
operator<< (std::ostream& output, const NamedContainer<C>&)
{
  // Enclose the entire output in braces
  output << '{';

  // Output the first value from a non-empty set
  // All values are given as ordered (index, value) pairs
  if (C::size)
    output << C::head_type::name();

  // Output the comma-separated subsequent values from a non-singleton
  // set
  if (C::size > 1)
    typename C::tail_set::ForEach()
      (NamedContainerOutputFunctor(output));

  output << '}';
  return output;
}


} // namespace MetaPhysicL

#endif // METAPHYSICL_PHYSICS_H

