//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

#include "http.h"

#include <atomic>
#include <memory>
#include <thread>

/**
 * Starts a webserver that an external process can connect to
 * in order to send JSON messages to control the solve
 */
class WebServerControl : public Control
{
public:
  static InputParameters validParams();

  WebServerControl(const InputParameters & parameters);

  ~WebServerControl();

  virtual void execute() override;

protected:
  /**
   * Internal method for starting the server
   */
  std::unique_ptr<std::thread> startServer();

  /**
   * @return Whether or not the server is currently waiting
   */
  bool currentlyWaiting() const { return _currently_waiting.load(); }

  /// Whether or not the Control is currently waiting
  std::atomic<bool> _currently_waiting;

  /// The port to run on
  const unsigned int _port;
  /// The server instance
  HttpServer _server;
  /// The server thread
  const std::unique_ptr<std::thread> _server_thread;

  /// The Real data that we accumulate from the server
  std::map<std::string, Real> _real_data;
  /// The Real data that we accumulate from the server
  std::map<std::string, std::string> _string_data;
  /// The std::vector<Real> data that we accumulate from the server
  std::map<std::string, std::vector<Real>> _vec_real_data;
  /// The std::vector<std::string> data that we accumulate from the server
  std::map<std::string, std::vector<std::string>> _vec_string_data;
  /// Mutex to prevent threaded writes to _real_data and _vec_real_data
  std::mutex _data_mutex;
};
