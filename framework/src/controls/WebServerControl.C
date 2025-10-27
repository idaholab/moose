//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControl.h"
#include "FEProblemBase.h"
#include "MooseApp.h"

#include "minijson/minijson.h"

registerMooseObject("MooseApp", WebServerControl);

InputParameters
WebServerControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Starts a webserver for sending/receiving JSON messages to get data "
                             "and control a running MOOSE calculation");
  params.addParam<unsigned int>("port",
                                "The port to listen on; must provide either this or 'file_socket'");
  params.addParam<FileName>(
      "file_socket",
      "The path to the unix file socket to listen on; must provide either this or 'port'");
  params.addParam<Real>("initial_client_timeout",
                        10,
                        "Time in seconds to allow the client to begin communicating on init; if "
                        "this time is surpassed the run will be killed");
  params.addParam<Real>("client_timeout",
                        10,
                        "Time in seconds to allow the client to communicate; if this time is "
                        "surpassed the run will be killed");
  return params;
}

WebServerControl::WebServerControl(const InputParameters & parameters)
  : Control(parameters),
    _port(queryParam<unsigned int>("port")),
    _file_socket(queryParam<FileName>("file_socket")),
    _initial_client_timeout(getParam<Real>("initial_client_timeout")),
    _client_timeout(getParam<Real>("client_timeout"))
{
  if (!_port && !_file_socket)
    mooseError("You must provide either the parameter 'port' or 'file_socket' to designate where "
               "to listen");
  if (_port && _file_socket)
    paramError("port", "Cannot provide both 'port' and 'file_socket'");
}

WebServerControl::~WebServerControl()
{
  // Stop server if running; make sure this DOESN'T throw!
  stopServer();
}

void
WebServerControl::startServer(const Moose::PassKey<WebServerControlStartServerAction>)
{
  // Only start on rank 0
  if (processor_id() == 0)
  {
    mooseAssert(_server_weak_ptr.expired(), "Server is already started");
    mooseAssert(!_server_thread_ptr, "Server thread is already listening");
    mooseAssert(_port || _file_socket, "Neither set");

    // Setup and start the server
    {
      // Instantiate the server but don't start it, we need
      // to setup all of the actions
      auto server_ptr = std::make_shared<HttpServer>();
      // Give the control a weak_ptr to the server, as it will
      // really be owned by the server thread
      _server_weak_ptr = server_ptr;

      // Add all of the actions
      addServerActionsInternal();

      // Post message about server start
      {
        std::ostringstream message;
        message << "Starting server on ";
        if (_port)
          message << "port " << *_port;
        else if (_file_socket)
          message << "file socket " << *_file_socket;
        outputMessage(message.str());
      }

      // Start the server thread, giving the thread the server
      // shared_ptr, as the control only has a weak_ptr to the server
      _server_thread_ptr = std::make_unique<std::thread>(
          [server_ptr](const auto port, const auto file_socket)
          {
            mooseAssert(server_ptr, "Null server");
            auto & server = *server_ptr;
            mooseAssert(port || file_socket, "Neither provided");

            try
            {
              if (port)
                server.startListening(uint16_t(*port));
              else
                server.startListening(*file_socket);
            }
            catch (std::exception & e)
            {
              std::cerr << "Server failed with exception: " << e.what() << std::endl;
            }
          },
          _port,
          _file_socket);
    }

    // Wait for the client to call /initialize
    {
      // Output that we're waiting for the client
      outputMessage("Waiting for client to initialize...");

      // To output initialization time
      const auto start = std::chrono::steady_clock::now();

      while (!isClientInitialized())
      {
        // Kill command sent before initialize
        if (isKillRequested())
        {
          stopServer();
          mooseError("Client sent kill command; exiting");
        }

        // Make sure we haven't reached the timeout
        const auto now = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration<double>(now - start).count();
        if (elapsed > _initial_client_timeout)
          mooseError(clientTimeoutErrorMessage(
              _initial_client_timeout, "initial_client_timeout", "during initialization"));

        // Poll until the next check
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }

      // Assign a start time for the timeout thread, considering
      // client initialization to be a poke
      clientPoke();

      // Output that the client has initialized
      const auto info = getClientInfo();
      outputClientTiming(
          "\"" + info.name + "\" from " + info.user + "@" + info.host + " initialized", start);
    }
  }

  // Let the remaining ranks wait until rank 0 is done
  _communicator.barrier();
}

void
WebServerControl::execute()
{
  // If simulation is requested to terminate, do not go through this control
  if (_fe_problem.isSolveTerminationRequested())
    return;

  // For outputting the time spent waiting
  const auto start = std::chrono::steady_clock::now();

  // Needed to broadcast all of the types and names of data that we have received on rank 0
  // so that we can construct the same objects on the other ranks to receive the data and
  // set the same values
  std::vector<std::pair<std::string, std::string>> name_and_types;

  // Need to also broadcast whether or not to terminate the solve on the timestep
  bool terminate_solve = false; // Set value to avoid compiler warnings

  // Wait for the server on rank 0 to be done
  if (processor_id() == 0)
  {
    TIME_SECTION("execute()", 3, "WebServerControl waiting for input")
    outputMessage("Waiting for client to continue...");

    setCurrentlyWaiting(true);

    // While waiting, yield so the server has time to run. Check for client
    // timeouts and a kill command.
    while (isCurrentlyWaiting())
    {
      // Kill command sent
      if (isKillRequested())
      {
        stopServer();
        mooseError("Client sent kill command; exiting");
      }

      // Check client timeout
      const auto last = std::chrono::milliseconds(_last_client_poke.load());
      const auto now = std::chrono::steady_clock::now().time_since_epoch();
      const auto elapsed = std::chrono::duration<double>(now - last).count();
      if (elapsed > _client_timeout)
      {
        stopServer();
        mooseError(clientTimeoutErrorMessage(_client_timeout, "client_timeout"));
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Output waiting time
    outputClientTiming("continued", start);

    for (const auto & value_ptr : _controlled_values)
      name_and_types.emplace_back(value_ptr->name(), value_ptr->type());

    terminate_solve = isTerminateRequested();
    setTerminateRequested(false);
    _terminate_requested.store(false);
  }

  // All processes need to wait
  _communicator.barrier();

  // Construct the values on other processors to be received into so that
  // they're parallel consistent
  comm().broadcast(name_and_types);
  if (processor_id() != 0)
    for (const auto & [name, type] : name_and_types)
      _controlled_values.emplace_back(Moose::WebServerControlTypeRegistry::build(type, name));

  // Set all of the values
  for (auto & value_ptr : _controlled_values)
  {
    try
    {
      value_ptr->setControllableValue(*this);
    }
    catch (...)
    {
      mooseError("Error setting '",
                 value_ptr->type(),
                 "' typed value for parameter '",
                 value_ptr->name(),
                 "'; it is likely that the parameter has a different type");
    }
  }

  _controlled_values.clear();

  // Set solve terminate on all ranks, if requested
  _communicator.broadcast(terminate_solve);
  if (terminate_solve)
    _fe_problem.terminateSolve();
}

template <WebServerControl::RequestMethod method>
void
WebServerControl::addServerAction(
    const std::string & path,
    std::function<WebServerControl::Response(const WebServerControl::Request &,
                                             WebServerControl &)> && action,
    const WebServerControl::ServerActionOptions & options)
{
  static_assert(method == RequestMethod::GET || method == RequestMethod::POST, "Unknown method");

  auto server_ptr = _server_weak_ptr.lock();
  if (!server_ptr || _server_thread_ptr)
    mooseError("addServerAction(): Can only call during addServerActions()");
  auto & server = *server_ptr;

  // Capture a weak pointer to the WebServerControl so that this action
  // knows if the control is available or not
  const std::weak_ptr<WebServerControl> control_weak_ptr =
      std::dynamic_pointer_cast<WebServerControl>(getSharedPtr());

  // Build the action that we'll actually pass to the web server.
  // Here, is very important that we capture everything by value
  // so that we don't rely on state from the control.
  auto full_action =
      [control_weak_ptr, path, options, action](const HttpRequest & http_request) -> HttpResponse
  {
    // Helper for returning an error
    const auto error = [](const std::string & error,
                          const unsigned int status_code = 400) -> HttpResponse
    {
      miniJson::Json::_object response;
      response["error"] = error;
      return HttpResponse{status_code, response};
    };

    // Only do work here if we have access to the control;
    // if this lock fails, it means that it has been destructed
    // and there isn't anything we can do
    if (auto control_ptr = control_weak_ptr.lock())
    {
      auto & control = *control_ptr;

      // Setup the request to be passed to the user function,
      // reformatting the HttpRequest as our own request
      // so we can change the underlying library if needed
      Request request;
      const auto & json_keys = options.getRequiredJSONKeys();
      // Load JSON data if it exists into nlohmann::json
      if (!http_request.json().isNull())
      {
        // JSON data exists but was not needed
        if (json_keys.empty())
          return error("Request should not have JSON");

        // Serialize from minijson to nlohmann::json
        const auto serialized = http_request.json().serialize();
        const auto deserialized = nlohmann::json::parse(serialized);

        // Check for required key(s)
        for (const auto & key : json_keys)
          if (!deserialized.contains(key))
            return error("Missing required key '" + key + "' in JSON");

        // And set in the request
        request.setJSON(deserialized, {});
      }
      // JSON data does not exist but it was needed
      else if (json_keys.size())
        return error("Request should have JSON");

      // Option requires initialization
      if (options.require_initialized && !control.isClientInitialized())
        return error("Client has not initialized the control");

      // Option requires waiting
      if (options.require_waiting && !control.isCurrentlyWaiting())
        return error("Control is not currently waiting for data");

      // Call the action method to act on the request
      Response response;
      try
      {
        response = action(request, control);
      }
      catch (const std::exception & e)
      {
        return error(e.what());
      }

      // Has an error, return that instead
      if (response.hasError())
        return error(response.getError(), response.getStatusCode());

      // No JSON data, just a status code
      if (!response.hasJSON())
        return HttpResponse{response.getStatusCode()};

      // Has JSON data, convert from nlohmann::json to miniJson
      const auto serialized = response.getJSON().dump();
      std::string parse_error;
      const auto json = miniJson::Json::parse(serialized, parse_error);
      mooseAssert(parse_error.empty(), "Should be empty");
      return HttpResponse{response.getStatusCode(), json};
    }

    // Failed to capture weak_ptr to WebServerControl
    return error("Control is no longer available");
  };

  auto & when = *server.when("/" + path);
  if constexpr (method == RequestMethod::GET)
    when.requested(full_action);
  else
    when.posted(full_action);
}

WebServerControl::ClientInfo
WebServerControl::getClientInfo() const
{
  std::lock_guard lock(_client_info_lock);
  if (!_client_info)
    mooseError("WebServerControl::getClientInfo(): Client info is not set yet");
  return *_client_info;
}

void
WebServerControl::outputMessage(const std::string & message) const
{
  _console << typeAndName() << ": " << message << std::endl;
}

void
WebServerControl::addServerActionsInternal()
{
  //
  // -- General actions ------------------------------------------------------------
  //

  // GET /check: Helper for checking if the server is running
  // Requires waiting: no
  // Return code: 200
  // Return JSON data: none
  {
    const auto check = [](const Request &, WebServerControl &) -> Response
    { return Response{200}; };

    ServerActionOptions options;
    options.require_waiting = false;
    options.require_initialized = false;

    addServerAction<RequestMethod::GET>("check", check, options);
  }

  // GET /continue: Tell the simulation to continue
  // Requires waiting: yes
  // Return code: 200
  // Return JSON data:
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto set_continue = [](const Request &, WebServerControl & control) -> Response
    {
      control.setCurrentlyWaiting(false);
      return Response{200};
    };

    addServerAction<RequestMethod::GET>("continue", set_continue);
  }

  // GET /initialize: Initializes the communication with the client
  // Requires waiting: no
  // POST JSON data:
  //    'name': string
  //        The name of the client
  //    'host': string
  //        The name the client host
  //    'user': string
  //        The name of the client user
  // Return code: 200
  // Return JSON data:
  //    'flags': list[string]
  //         The execute on flags
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto initialize = [](const Request & req, WebServerControl & control) -> Response
    {
      if (control.isClientInitialized())
        return ErrorResponse("Initialize has already been called");

      // Store the information received in the client info
      const auto & json = req.getJSON();
      ClientInfo client_info;
      const auto name = convertJSON<std::string>(json, "name");
      const auto host = convertJSON<std::string>(json, "host");
      const auto user = convertJSON<std::string>(json, "user");
      client_info.data = json;
      control.setClientInfo(client_info);

      // Capture the sorted exceute on flags
      std::set<std::string> flags_sorted;
      for (const auto & it : control.getParam<ExecFlagEnum>("execute_on"))
        flags_sorted.insert(it);
      const std::vector<std::string> flags(flags_sorted.begin(), flags_sorted.end());

      // Send back context about the control
      nlohmann::json response_json;
      response_json["control_name"] = control.name();
      response_json["control_type"] = control.type();
      response_json["execute_on_flags"] = flags_sorted;

      // Set initialized
      control.setClientInitialized();

      return Response{200, response_json};
    };

    ServerActionOptions options;
    options.require_waiting = false;
    options.require_initialized = false;
    options.requireJSONKeys({"host", "name", "user"});

    addServerAction<RequestMethod::POST>("initialize", initialize, options);
  }

  // GET /poke: "Poke" the server; used for checking timeouts
  // Requires waiting: no
  // Return code: 200
  // Return JSON data: none
  {
    const auto poke = [](const Request &, WebServerControl & control) -> Response
    {
      control.clientPoke();
      return Response{200};
    };

    ServerActionOptions options;
    options.require_waiting = false;

    addServerAction<RequestMethod::GET>("poke", poke, options);
  }

  // GET /terminate: Tell FEProblemBase to terminate the solve
  // Requires waiting: yes
  // Return code: 200
  // Return JSON data:
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto terminate = [](const Request &, WebServerControl & control) -> Response
    {
      control.setTerminateRequested();
      control.setCurrentlyWaiting(false);
      return Response{200};
    };

    addServerAction<RequestMethod::GET>("terminate", terminate);
  }

  // GET /waiting: Check if waiting and the waiting exec flag if it exists
  // Requires waiting: no
  // Return code: 200
  // Return JSON data:
  //    'waiting': bool
  //         Whether or not the control is waiting
  //     'execute_on_flag': string, optional
  //         Only exists if waiting=true, the execute flag that is being waited on
  {
    const auto waiting = [](const Request &, WebServerControl & control) -> Response
    {
      nlohmann::json response_json;
      if (control.isCurrentlyWaiting())
      {
        response_json["waiting"] = true;
        response_json["execute_on_flag"] =
            static_cast<std::string>(control._fe_problem.getCurrentExecuteOnFlag());
      }
      else
        response_json["waiting"] = false;

      return Response{200, response_json};
    };

    ServerActionOptions options;
    options.require_waiting = false;

    addServerAction<RequestMethod::GET>("waiting", waiting, options);
  }

  // GET /kill: Tell the client poll thread to kill
  // Requires waiting: no
  {
    const auto kill = [](const Request &, WebServerControl & control) -> Response
    {
      control.setKillRequested();
      return Response{200};
    };

    addServerAction<RequestMethod::GET>("kill", kill);
  }

  //
  // -- Get actions ----------------------------------------------------------------
  //

  // GET /get/dt: Get current simulation timestep size
  // Requires waiting: yes
  // Return code: 200
  // Return JSON data:
  //    'dt': double
  //         Current timestep size
  // Return JSON data:
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto get_dt = [](const Request &, WebServerControl & control) -> Response
    {
      nlohmann::json response_json;
      response_json["dt"] = control._fe_problem.dt();
      return Response{200, response_json};
    };

    addServerAction<RequestMethod::GET>("get/dt", get_dt);
  }

  // POST /get/postprocessor: Get a postprocessor value
  // Requires waiting: yes
  // POST JSON data:
  //    'name': string
  //        The name of the Postprocessor
  // Return code: 200
  // Return JSON data:
  //    'value': double
  //         The postprocessor value
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto get_postprocessor = [](const Request & req, WebServerControl & control) -> Response
    {
      // Get the postprocessor name
      const auto name = convertJSON<std::string>(req.getJSON(), "name");

      // Postprocessor should exist
      if (!control.hasPostprocessorByName(name))
        return ErrorResponse("Postprocessor '" + name + "' not found");

      nlohmann::json response_json;
      response_json["value"] = control.getPostprocessorValueByName(name);
      return Response{200, response_json};
    };

    ServerActionOptions options;
    options.requireJSONKey("name");

    addServerAction<RequestMethod::POST>("get/postprocessor", get_postprocessor, options);
  }

  // POST /get/reporter: Get a Reporter value
  // Requires waiting: yes
  // POST JSON data:
  //    'name': string
  //        The name of the Reporter value (object_name/value_name)
  // Return code: 200
  // Return JSON data:
  //    'value': any
  //         The reporter value
  // Return JSON data:
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto get_reporter = [](const Request & req, WebServerControl & control) -> Response
    {
      // Get the reporter name
      const auto name = convertJSON<std::string>(req.getJSON(), "name");
      if (!ReporterName::isValidName(name))
        return ErrorResponse("Name '" + name + "' not a valid reporter value name");
      const auto rname = ReporterName(name);

      // Reporter should exist
      if (!control.hasReporterValueByName(rname))
        return ErrorResponse("Reporter value '" + name + "' was not found");

      // Store the reporter value
      nlohmann::json response_json;
      control.getReporterContextBaseByName(rname).store(response_json["value"]);

      return Response{200, response_json};
    };

    ServerActionOptions options;
    options.requireJSONKey("name");

    addServerAction<RequestMethod::POST>("get/reporter", get_reporter, options);
  }

  // GET /get/time: Get current simulation time
  // Requires waiting: yes
  // Return code: 200
  // Return JSON data:
  //    'time': double
  //         Current time
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto get_time = [](const Request &, WebServerControl & control) -> Response
    {
      nlohmann::json response_json;
      response_json["time"] = control._fe_problem.time();
      return Response{200, response_json};
    };

    addServerAction<RequestMethod::GET>("get/time", get_time);
  }

  //
  // -- Set actions ----------------------------------------------------------------
  //

  // POST /set/controllable: Get a controllable parameter
  // Requires waiting: yes
  // POST JSON data:
  //    'name': string
  //        The path to the controllable data
  //    'value': any
  //        The value to set
  //    'type: string
  //        The C++ type of the controllable parameter
  // Return code: 201
  // Return JSON data:
  //    'error': string, optional
  //         The error (only set if an error occurred)
  {
    const auto set_controllable = [](const Request & req, WebServerControl & control) -> Response
    {
      const auto & json = req.getJSON();

      // Get the parameter type
      const auto type = convertJSON<std::string>(json, "type");
      if (!Moose::WebServerControlTypeRegistry::isRegistered(type))
        return ErrorResponse("Type '" + type +
                             "' not registered for setting a controllable parameter");

      // Get the parameter name
      const auto name = convertJSON<std::string>(json, "name");
      // Parameter should exist
      if (!control.hasControllableParameterByName(name))
        return ErrorResponse("Controllable parameter '" + name + "' not found");

      // 'value' must exist
      const auto value_it = json.find("value");
      if (value_it == json.end())
        return ErrorResponse("Missing 'value' entry");

      // Build the value (also does the parsing)
      {
        std::unique_ptr<ControlledValueBase> value;
        try
        {
          value = Moose::WebServerControlTypeRegistry::build(type, name, *value_it);
        }
        catch (std::exception & e)
        {
          return ErrorResponse("While parsing 'value': " + std::string(e.what()));
        }

        std::lock_guard<std::mutex> lock(control._controlled_values_mutex);
        control._controlled_values.emplace_back(std::move(value));
      }

      return Response{201};
    };

    ServerActionOptions options;
    options.requireJSONKeys({"name", "type", "value"});

    addServerAction<RequestMethod::POST>("set/controllable", set_controllable, options);
  }

  // Let derived classes add actions
  addServerActions();
}

void
WebServerControl::setClientInfo(const WebServerControl::ClientInfo & info)
{
  std::lock_guard lock(_client_info_lock);
  _client_info = info;
}

void
WebServerControl::clientPoke()
{
  const int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
  _last_client_poke.store(now);
}

void
WebServerControl::outputClientTiming(
    const std::string & message,
    const std::chrono::time_point<std::chrono::steady_clock> & start) const
{
  const auto now = std::chrono::steady_clock::now();
  const auto elapsed = std::chrono::duration<double>(now - start).count();

  std::ostringstream out;
  out << "Client " << message << " after " << std::fixed << std::setprecision(2) << elapsed
      << " seconds";
  outputMessage(out.str());
}

std::string
WebServerControl::clientTimeoutErrorMessage(
    const Real timeout,
    const std::string & timeout_param_name,
    const std::optional<std::string> & suffix /* = {} */) const
{
  std::ostringstream oss;
  oss << "The client timed out" << (suffix ? (" " + *suffix) : "") << "\nThe timeout is "
      << std::fixed << std::setprecision(2) << timeout << " seconds and is set by the '"
      << timeout_param_name << "' parameter";
  return oss.str();
}

void
WebServerControl::stopServer()
{
  // Only have something to do here if the server still exists
  // and the server thread was set; make sure this doesn't
  // throw because it is used in the destructor
  if (auto server_ptr = _server_weak_ptr.lock(); server_ptr && _server_thread_ptr)
  {
    try
    {
      server_ptr->shutdown();
      _server_thread_ptr->join();
      _server_thread_ptr.reset();
    }
    catch (std::exception & e)
    {
      std::cerr << "Server shutdown raised exception: " << e.what() << std::endl;
    }
  }
}

/// Explicitly instantiate the addServerAction method for the valid request types
///@{
template void WebServerControl::addServerAction<WebServerControl::RequestMethod::GET>(
    const std::string &,
    std::function<WebServerControl::Response(const WebServerControl::Request &,
                                             WebServerControl &)> &&,
    const WebServerControl::ServerActionOptions &);
template void WebServerControl::addServerAction<WebServerControl::RequestMethod::POST>(
    const std::string &,
    std::function<WebServerControl::Response(const WebServerControl::Request &,
                                             WebServerControl &)> &&,
    const WebServerControl::ServerActionOptions & options);
///@}

const nlohmann::json &
WebServerControl::Request::getJSON() const
{
  if (!hasJSON())
    throw std::runtime_error("Request does not contain JSON when it should");
  return *_json;
}

WebServerControl::Response::Response(const unsigned int status_code) : _status_code(status_code) {}

WebServerControl::Response::Response(const unsigned int status_code, const nlohmann::json & json)
  : _status_code(status_code), _json(json)
{
}

const nlohmann::json &
WebServerControl::Response::getJSON() const
{
  if (!_json)
    throw std::runtime_error("Response does not contain JSON when it should");
  return *_json;
}

const std::string &
WebServerControl::Response::getError() const
{
  if (!_error)
    ::mooseError("Does not have an error");
  return *_error;
}

WebServerControl::ErrorResponse::ErrorResponse(const std::string & error,
                                               const unsigned int status_code /* = 400 */)
  : WebServerControl::Response(status_code)
{
  setError(error);
}
