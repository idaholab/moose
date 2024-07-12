//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControl.h"

#include "minijson/minijson.h"

registerMooseObject("MooseApp", WebServerControl);

#define registerWebServerControlCombine1(X, Y) X##Y
#define registerWebServerControlCombine(X, Y) registerWebServerControlCombine1(X, Y)
#define registerWebServerControlScalar(T, json_type)                                               \
  static char registerWebServerControlCombine(wsc_scalar, __COUNTER__) =                           \
      WebServerControl::registerScalarType<T, json_type>(#T)
#define registerWebServerControlVector(T, json_type)                                               \
  static char registerWebServerControlCombine(wsc_vector, __COUNTER__) =                           \
      WebServerControl::registerVectorType<T, json_type>(#T)
#define registerWebServerControlScalarBool(T)                                                      \
  registerWebServerControlScalar(T, miniJson::JsonType::kBool)
#define registerWebServerControlScalarNumber(T)                                                    \
  registerWebServerControlScalar(T, miniJson::JsonType::kNumber)
#define registerWebServerControlScalarString(T)                                                    \
  registerWebServerControlScalar(T, miniJson::JsonType::kString)
#define registerWebServerControlVectorNumber(T)                                                    \
  registerWebServerControlVector(T, miniJson::JsonType::kNumber)
#define registerWebServerControlVectorString(T)                                                    \
  registerWebServerControlVector(T, miniJson::JsonType::kString)

// Registration of the types that we can accept in the web server for controlling parameters
registerWebServerControlScalarBool(bool);
registerWebServerControlScalarNumber(Real);
registerWebServerControlScalarNumber(int);
registerWebServerControlScalarString(std::string);
registerWebServerControlVectorNumber(Real);
registerWebServerControlVectorNumber(int);
registerWebServerControlVectorString(std::string);

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
  return params;
}

WebServerControl::WebServerControl(const InputParameters & parameters)
  : Control(parameters), _currently_waiting(false)
{
  const auto has_port = isParamValid("port");
  const auto has_file_socket = isParamValid("file_socket");
  if (!has_port && !has_file_socket)
    mooseError("You must provide either the parameter 'port' or 'file_socket' to designate where "
               "to listen");
  if (has_port && has_file_socket)
    paramError("port", "Cannot provide both 'port' and 'file_socket'");

  if (processor_id() == 0)
    startServer();
}

WebServerControl::~WebServerControl()
{
  if (_server)
  {
    _server->shutdown();
    _server_thread->join();
  }
}

void
WebServerControl::startServer()
{
  mooseAssert(processor_id() == 0, "Should only be started on rank 0");
  mooseAssert(!_server, "Server is already started");
  mooseAssert(!_server_thread, "Server thread is already listening");

  // Helper for returning an error response
  const auto error = [](const std::string & error)
  {
    miniJson::Json::_object response;
    response["error"] = error;
    return HttpResponse{400, response};
  };

  // Helper for getting a string from a json value with error checking
  const auto get_string =
      [&error](const auto & msg, const std::string & name, const std::string & description)
  {
    using result = std::variant<std::string, HttpResponse>;
    const auto it = msg.find(name);
    if (it == msg.end())
      return result(
          error("The entry '" + name + "' is missing which should contain the " + description));
    const auto & value = it->second;
    if (!value.isString())
      return result(error("The entry '" + name + "' which should contain the " + description));
    return result(value.toString());
  };

  // Helper for getting a string name from a json value with error checking
  const auto get_name = [&get_string](const auto & msg, const std::string & description)
  { return get_string(msg, "name", "name of the " + description); };

  // Helper for requiring that the control is waiting
  // Note that this is very hard to test unless we want to add sleeps
  const auto require_waiting = [&error](auto & control)
  {
    using result = std::optional<HttpResponse>;
    if (!control.currentlyWaiting())
      return result(error("This control is not currently waiting for data"));
    return result{};
  };

  const auto require_parameters = [&error](const auto & msg, const std::set<std::string> & params)
  {
    using result = std::optional<HttpResponse>;
    for (const auto & key_value_pair : msg)
      if (!params.count(key_value_pair.first))
        return result(error("The key '" + key_value_pair.first + "' is unused"));
    return result{};
  };

  _server = std::make_unique<HttpServer>();

  // GET /check, returns code 200
  _server->when("/check")->requested([](const HttpRequest & /*req*/) { return HttpResponse{200}; });

  // GET /waiting, returns code 200 on success and JSON:
  //  'waiting' (bool): Whether or not the control is waiting
  //  'execute_on_flag' (string): Only exists if waiting=true, the execute
  //                              flag that is being waited on
  _server->when("/waiting")
      ->requested(
          [this](const HttpRequest & /*req*/)
          {
            miniJson::Json::_object res_json;
            if (this->_currently_waiting.load())
            {
              res_json["waiting"] = true;
              res_json["execute_on_flag"] =
                  static_cast<std::string>(this->_fe_problem.getCurrentExecuteOnFlag());
            }
            else
              res_json["waiting"] = false;

            return HttpResponse{200, res_json};
          });

  // POST /get/postprocessor, with data:
  //   'name' (string): The name of the Postprocessor
  // Returns code 200 on success and JSON:
  //   'value' (double): The postprocessor value
  _server->when("/get/postprocessor")
      ->posted(
          [this, &error, &get_name, &require_waiting, &require_parameters](const HttpRequest & req)
          {
            const auto & msg = req.json().toObject();

            // Get the postprocessor name
            const auto name_result = get_name(msg, "postprocessor to retrieve");
            if (const auto response = std::get_if<HttpResponse>(&name_result))
              return *response;
            const auto & name = std::get<std::string>(name_result);

            // Should only have a name
            if (const auto response = require_parameters(msg, {"name"}))
              return *response;
            // Should be waiting for data
            if (const auto response = require_waiting(*this))
              return *response;
            // Postprocessor should exist
            if (!this->hasPostprocessorByName(name))
              return error("The postprocessor '" + name + "' was not found");

            miniJson::Json::_object res_json;
            res_json["value"] = getPostprocessorValueByName(name);
            return HttpResponse{200, res_json};
          });

  // POST /set/controllable, with data:
  //   'name' (string): The path to the controllable data
  //   'value': The data to set
  //   'type' (string): The C++ type of the controllable data to set
  // Returns code 201 on success and JSON:
  //   'error' (string): The error (only set if an error occurred)
  _server->when("/set/controllable")
      ->posted(
          [this, &error, &get_string, &get_name, &require_waiting, &require_parameters](
              const HttpRequest & req)
          {
            const auto & msg = req.json().toObject();

            // Should only have a name, type, and value
            if (const auto response = require_parameters(msg, {"name", "type", "value"}))
              return *response;
            // Should be waiting for data
            if (const auto response = require_waiting(*this))
              return *response;

            // Get the parameter type
            const auto type_result = get_string(msg, "type", "type of the parameter");
            if (const auto response = std::get_if<HttpResponse>(&type_result))
              return *response;
            const auto & type = std::get<std::string>(type_result);
            if (!Moose::WebServerControlTypeRegistry::isRegistered(type))
              return error("The type '" + type +
                           "' is not registered for setting a controllable parameter");

            // Get the parameter name
            const auto name_result = get_name(msg, "name of the parameter to control");
            if (const auto response = std::get_if<HttpResponse>(&name_result))
              return *response;
            const auto & name = std::get<std::string>(name_result);
            // Parameter should exist
            if (!this->hasControllableParameterByName(name))
              return error("The controllable parameter '" + name + "' was not found");

            // Get the parameter value
            const auto value_it = msg.find("value");
            if (value_it == msg.end())
              return error(
                  "The entry 'value' is missing which should contain the value of the parameter");
            const auto & json_value = value_it->second;

            // Build the value (also does the parsing)
            {
              std::unique_ptr<ValueBase> value;
              std::lock_guard<std::mutex> lock(this->_controlled_values_mutex);
              try
              {
                value = Moose::WebServerControlTypeRegistry::build(type, name, json_value);
              }
              catch (ValueBase::Exception & e)
              {
                return error("While parsing 'value': " + std::string(e.what()));
              }
              _controlled_values.emplace_back(std::move(value));
            }

            return HttpResponse{201};
          });

  // GET /continue, Returns code 200
  _server->when("/continue")
      ->requested(
          [this, &error](const HttpRequest &)
          {
            if (this->_currently_waiting.load())
            {
              this->_currently_waiting.store(false);
              return HttpResponse{200};
            }

            // Not currently waiting
            return error("The control is not currently waiting");
          });

  _server_thread = std::make_unique<std::thread>(
      [this]
      {
        if (this->isParamValid("port"))
        {
          const uint16_t port = this->getParam<unsigned int>("port");
          try
          {
            _server->startListening(port);
          }
          catch (...)
          {
            this->mooseError("Failed to start the webserver; it is likely that the port ",
                             port,
                             " is not available");
          }
        }
        else if (this->isParamValid("file_socket"))
        {
          const auto & file_socket = this->getParam<FileName>("file_socket");
          _server->startListening(file_socket);
        }
      });
}

void
WebServerControl::execute()
{
  // Needed to broadcast all of the types and names of data that we have received on rank 0
  // so that we can construct the same objects on the other ranks to receive the data and
  // set the same values
  std::vector<std::pair<std::string, std::string>> name_and_types;

  // Wait for the server on rank 0 to be done
  if (processor_id() == 0)
  {
    TIME_SECTION("execute()", 3, "WebServerControl waiting for input")

    _currently_waiting.store(true);

    // While waiting, yield so the server has time to run
    while (_currently_waiting.load())
      std::this_thread::yield();

    for (const auto & value_ptr : _controlled_values)
      name_and_types.emplace_back(value_ptr->name(), value_ptr->type());
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
}

std::string
WebServerControl::stringifyJSONType(const miniJson::JsonType & json_type)
{
  if (json_type == miniJson::JsonType::kNull)
    return "empty";
  if (json_type == miniJson::JsonType::kBool)
    return "bool";
  if (json_type == miniJson::JsonType::kNumber)
    return "number";
  if (json_type == miniJson::JsonType::kString)
    return "string";
  if (json_type == miniJson::JsonType::kArray)
    return "array";
  if (json_type == miniJson::JsonType::kObject)
    return "object";
  ::mooseError("WebServerControl::stringifyJSONType(): Unused JSON value type");
}
