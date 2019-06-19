//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/***
 * Streams all of the given arguments into the given stream
 */
template <class StreamType>
void
streamArguments(StreamType &)
{
}

// Recursive parameter pack expansion
template <class StreamType, typename T, typename... Args>
void
streamArguments(StreamType & ss, T && val, Args &&... args)
{
  ss << val;
  streamArguments(ss, std::forward<Args>(args)...);
}

// Base case for Tuple expansion recursive method
template <std::size_t I = 0, class StreamType, typename... Args>
inline typename std::enable_if<I == sizeof...(Args), void>::type
streamArguments(StreamType & /*ss*/, std::tuple<Args...> /*args*/)
{
}

// Recursive method for tuple expansion
// clang-format off
template <std::size_t I = 0, class StreamType, typename... Args>
inline typename std::enable_if<I < sizeof...(Args), void>::type
streamArguments(StreamType & ss, std::tuple<Args...> args)
{
  ss << std::get<I>(args);
  streamArguments<I + 1, StreamType, Args...>(ss, args);
}
// clang-format on
