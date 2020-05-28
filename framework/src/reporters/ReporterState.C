//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterState.h"

ReporterContextBase::ReporterContextBase(const libMesh::ParallelObject & other)
  : libMesh::ParallelObject(other)
{
}

// template <typename T>
// ReporterState<T>::ReporterState(T & value) //, T & value_old)
//    : ReporterStateBase(), _values(1, &value)     //, _value_old(value_old)
//{
//}
//
// template <typename T>
// T &
// ReporterState<T>::getValue(const std::size_t time_index) const
//{
//  mooseAssert(time_index < _values.size(), "Invalid time index.");
//  return *_values[time_index];
//}
//
// template class ReporterState<int>;
// template class ReporterState<Real>;
// template class ReporterState<std::vector<Real>>;
// template class ReporterState<std::string>;
//
//
// template <typename T>
// ReporterBroadcastState<T>::ReporterBroadcastState(T & value) :
//    ReporterState<T>(value)
//{
//}
//
///*
// template <typename T>
// void ReporterBroadcastState<T>::finalize(const libMesh::Parallel::Communicator & comm)
//{
//  std::cout << "ReporterBroadcastState::finalize" << std::endl;
//  comm.broadcast(this->_value);
//}
//*/
//
////template class ReporterBroadcastState<int>;
// template class ReporterBroadcastState<Real>;
////template class ReporterBroadcastState<std::vector<Real>>;
////template class ReporterBroadcastState<std::string>;
