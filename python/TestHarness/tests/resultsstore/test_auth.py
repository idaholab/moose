# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.auth."""

import os
from tempfile import NamedTemporaryFile
from unittest import TestCase
from unittest.mock import patch

import yaml
from TestHarness.resultsstore.auth import (
    Authentication,
    has_authentication,
    load_authentication,
)

VAR_PREFIX = "FOO"
DEFAULT_ARGS = {"host": "HOSTNAME", "username": "USER", "password": "PASS"}
DEFAULT_ENV = {f"{VAR_PREFIX}_AUTH_{k.upper()}": v for k, v in DEFAULT_ARGS.items()}


class TestAuth(TestCase):
    """Test TestHarness.resultsstore.auth."""

    def test_authentication(self):
        """Test Authentication."""
        auth = Authentication(**DEFAULT_ARGS)
        for k, v in DEFAULT_ARGS.items():
            self.assertEqual(getattr(auth, k), v)
        self.assertIsNone(auth.port)

    def test_authentication_with_port(self):
        """Test Authentication with a port."""
        port = 1234
        auth = Authentication(**DEFAULT_ARGS, port=port)
        self.assertEqual(auth.port, port)

    def test_load_authentication_env(self):
        """Test load_authentication() from environment."""
        with patch.dict(os.environ, DEFAULT_ENV, clear=False):
            auth = load_authentication(VAR_PREFIX)
            self.assertTrue(has_authentication(VAR_PREFIX))
        for k, v in DEFAULT_ARGS.items():
            self.assertEqual(getattr(auth, k), v)

    def test_load_authentication_env_with_port(self):
        """Test load_authentication() from environment."""
        port = 1234
        auth_env = {**DEFAULT_ENV, f"{VAR_PREFIX}_AUTH_PORT": str(port)}
        with patch.dict(os.environ, auth_env, clear=False):
            auth = load_authentication(VAR_PREFIX)
            self.assertTrue(has_authentication(VAR_PREFIX))
        self.assertEqual(auth.port, port)

    def test_load_authentication_env_not_all(self):
        """Test load_authentication() with not all variables set."""
        auth_env = {**DEFAULT_ENV}

        del auth_env[f"{VAR_PREFIX}_AUTH_HOST"]
        with (
            patch.dict(os.environ, auth_env, clear=False),
            self.assertRaisesRegex(ValueError, "must be set for authentication"),
        ):
            load_authentication(VAR_PREFIX)

        del auth_env[f"{VAR_PREFIX}_AUTH_USERNAME"]
        with (
            patch.dict(os.environ, auth_env, clear=False),
            self.assertRaisesRegex(ValueError, "must be set for authentication"),
        ):
            load_authentication(VAR_PREFIX)

    def test_load_authentication_file(self):
        """Test load_authentication from file."""
        with NamedTemporaryFile() as auth_file:
            with open(auth_file.name, "w") as f:
                yaml.safe_dump(DEFAULT_ARGS, f)

            auth_env = {f"{VAR_PREFIX}_AUTH_FILE": auth_file.name}
            with patch.dict(os.environ, auth_env, clear=False):
                auth = load_authentication(VAR_PREFIX)

        for k, v in DEFAULT_ARGS.items():
            self.assertEqual(getattr(auth, k), v)

    def test_load_authentication_file_with_port(self):
        """Test load_authentication from file with a port."""
        port = 1234
        auth_vars = {**DEFAULT_ARGS, "port": port}

        with NamedTemporaryFile() as auth_file:
            with open(auth_file.name, "w") as f:
                yaml.safe_dump(auth_vars, f)

            auth_env = {f"{VAR_PREFIX}_AUTH_FILE": auth_file.name}
            with patch.dict(os.environ, auth_env, clear=False):
                auth = load_authentication(VAR_PREFIX)

        self.assertEqual(auth.port, port)

    def test_load_authentication_file_fail(self):
        """Test catching an exception when loading by file."""
        file = "/foo/bar"
        auth_env = {f"{VAR_PREFIX}_AUTH_FILE": file}
        with (
            patch.dict(os.environ, auth_env, clear=False),
            self.assertRaisesRegex(
                Exception, f"Failed to load credentials from '{file}'"
            ),
        ):
            load_authentication(VAR_PREFIX)

    def test_load_authentication_none(self):
        """Test load_authentication() not loading from env for file."""
        self.assertIsNone(load_authentication(VAR_PREFIX))
