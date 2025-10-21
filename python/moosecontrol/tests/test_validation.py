# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

from requests import HTTPError
from typing import Any
from unittest import TestCase

from common import FAKE_URL, fake_response, setup_moose_python_path

setup_moose_python_path()

from moosecontrol.exceptions import BadStatus, UnexpectedResponse, WebServerControlError
from moosecontrol.validation import (
    check_response_data,
    process_response,
    WebServerControlResponse,
)


def fake_ws_response(**kwargs) -> WebServerControlResponse:
    return process_response(fake_response(**kwargs))


class TestValidation(TestCase):
    """
    Tests moosecontrol.validation.
    """

    def test_process_response_no_data(self):
        """
        Tests process_response() with a success and no data.
        """
        ws_response = fake_ws_response()
        self.assertEqual(ws_response.response.status_code, 200)
        self.assertFalse(ws_response.has_data())

    def test_process_response_with_data(self):
        """
        Tests process_response() with a success with data.
        """
        data = {"foo": "bar"}
        ws_response = fake_ws_response(data=data)

        self.assertEqual(ws_response.response.status_code, 200)
        self.assertEqual(ws_response.data, data)

    def test_process_response_require_status(self):
        """
        Tests process_response() with require_status set.
        """
        response = fake_response(status_code=404)

        with self.assertRaises(BadStatus) as e:
            process_response(response, require_status=200)

        self.assertEqual(
            str(e.exception), f"Request {FAKE_URL} status 404 != expected 200"
        )
        self.assertEqual(e.exception.response.status_code, 404)

    def test_process_response_error(self):
        """
        Tests process_response() with the 'error' field set in the
        JSON data.
        """
        error = "foo"
        data = {"error": error}
        response = fake_response(data=data)

        with self.assertRaises(WebServerControlError) as e:
            process_response(response)

        self.assertEqual(str(e.exception), f"Request to {FAKE_URL}: {error}")
        self.assertEqual(e.exception.error, error)

    def test_process_response_raise_for_status(self):
        """
        Tests process_response() throwing via raise_for_status().
        """
        with self.assertRaises(HTTPError):
            process_response(fake_response(status_code=404))

    def test_check_response_data_ok(self):
        """
        Tests check_response_data() with OK results.
        """
        # Only required data
        data = {
            "required_int": 0,
            "required_float": 1.0,
        }
        response = fake_ws_response(data=data)
        check_response_data(
            response,
            [("required_int", int), ("required_float", float)],
        )

        # Add optional, but not there
        check_response_data(
            response,
            [("required_int", int), ("required_float", float)],
            [("optional_str", str), ("optional_int", int)],
        )

        # With optional and some is there
        data.update({"optional_str": "foo"})
        response = fake_ws_response(data=data)
        check_response_data(
            response,
            [("required_int", int), ("required_float", float)],
            [("optional_str", str), ("optional_int", int)],
        )

    def test_check_response_any(self):
        """
        Tests check_response_data() with the Any type.
        """
        data = {
            "value": 0,
        }
        response = fake_ws_response(data=data)
        check_response_data(response, [("value", Any)])

    def test_check_response_multiple_types(self):
        """
        Tests check_response_data() with keys that could
        be multiple types.
        """
        data = {"required": 0, "optional": "foo"}
        response = fake_ws_response(data=data)
        check_response_data(
            response, [("required", (str, int)), ("optional", (int, str))]
        )

    def test_check_response_no_data(self):
        """
        Tests check_response_data() with no data.
        """
        response = fake_ws_response()
        with self.assertRaises(UnexpectedResponse) as e:
            check_response_data(response, [])
        self.assertEqual(
            str(e.exception), f"Response from {FAKE_URL} does not contain data"
        )

    def test_check_response_data_unexpected_keys(self):
        """
        Tests check_response_data() with unexpected keys.
        """
        key = "unexpected"
        data = {key: "foo"}
        response = fake_ws_response(data=data)
        with self.assertRaises(UnexpectedResponse) as e:
            check_response_data(response, [])
        self.assertEqual(
            str(e.exception), f"Response from {FAKE_URL} has unexpected key(s): {key}"
        )

    def test_check_response_data_missing_keys(self):
        """
        Tests check_response_data() with missing keys.
        """
        key = "missing"
        data = {"value": "foo"}
        response = fake_ws_response(data=data)
        with self.assertRaises(UnexpectedResponse) as e:
            check_response_data(response, [("value", str), (key, int)])
        self.assertEqual(
            str(e.exception), f"Response from {FAKE_URL} has missing key(s): {key}"
        )

    def test_check_response_data_unexpected_type(self):
        """
        Tests check_response_data() with an unexpected type.
        """
        key = "key"
        data = {key: 0.0}
        response = fake_ws_response(data=data)
        with self.assertRaises(UnexpectedResponse) as e:
            check_response_data(response, [(key, int)])
        self.assertEqual(
            str(e.exception),
            f'Response from {FAKE_URL} key "{key}" has unexpected type "float"',
        )
