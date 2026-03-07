# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Exceptions for pycapabilities."""


class CapabilityException(Exception):
    """Common execption to be thrown when interacting with capabilities."""

    pass


class UnknownCapabilitiesException(CapabilityException):
    """Exception when capabilities that are unknown are provided."""

    def __init__(self, message: str, unknown_capabilities: list[str]):
        """
        Initialize the exception.

        Arguments:
        ---------
        message : str
            The exception message.
        unknown_capabilities : list[str]
            The capabilities that were unknown.

        """
        assert isinstance(message, str)
        assert isinstance(unknown_capabilities, list)
        assert all(isinstance(v, str) for v in unknown_capabilities)

        super().__init__(message)

        self.unknown_capabilities: list[str] = unknown_capabilities
        """The capabilities that are unknown."""
