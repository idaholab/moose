# * This file is part of the MOOSE framework
# * https://mooseframework.inl.gov
# *
# * All rights reserved, see COPYRIGHT for full restrictions
# * https://github.com/idaholab/moose/blob/master/COPYRIGHT
# *
# * Licensed under LGPL 2.1, please see LICENSE for details
# * https://www.gnu.org/licenses/lgpl-2.1.html

from copy import deepcopy
from numbers import Number
from unittest.mock import patch
from typing import Any, Optional, Tuple

from common import MooseControlTestCase, setup_moose_python_path, \
    mock_response, FAKE_URL
setup_moose_python_path()

from test_runners_baserunner import BaseRunnerTest

from moosecontrol import MooseControlNew
from moosecontrol.exceptions import ControlNotWaiting

MOOSECONTROL = 'moosecontrol.MooseControlNew'
BASERUNNER = 'moosecontrol.runners.BaseRunner'

FAKE_EXECUTE_ON_FLAG = 'foo'
FAKE_NAME = 'cool_name'

def patch_runner(name: str, **kwargs):
    """
    Convenience method for patching the BaseRunner.
    """
    return patch(f'moosecontrol.runners.BaseRunner.{name}', **kwargs)

class TestMooseControl(MooseControlTestCase):
    """
    Tests moosecontrol.moosecontrol.MooseControl.
    """
    def test_init(self):
        """
        Test __init__() with the required and default arguments.
        """
        runner = BaseRunnerTest()
        control = MooseControlNew(runner)
        self.assertEqual(control.runner, runner)

    def test_initialize(self):
        """
        Tests that initialize() calls the underlying runner's initialize().
        """
        runner = BaseRunnerTest()
        control = MooseControlNew(runner)
        with patch(f'{BASERUNNER}.initialize') as runner_initialize:
            control.initialize()
        runner_initialize.assert_called_once()

    def test_finalize(self):
        """
        Tests that finalize() calls the underlying runner's finalize().
        """
        runner = BaseRunnerTest()
        control = MooseControlNew(runner)
        with patch(f'{BASERUNNER}.finalize') as runner_finalize:
            control.finalize()
        runner_finalize.assert_called_once()

    def test_cleanup(self):
        """
        Tests that cleanup() calls the underlying runner's cleanup().
        """
        runner = BaseRunnerTest()
        control = MooseControlNew(runner)
        with patch(f'{BASERUNNER}.cleanup') as runner_cleanup:
            control.cleanup()
        runner_cleanup.assert_called_once()

    def test_context_manager(self):
        """
        Tests the MooseControl context manager, which should call
        initialize() on enter and finalize() on exit.
        """
        runner = BaseRunnerTest()

        def action():
            with MooseControlNew(runner) as cm:
                self.assertIsInstance(cm.control, MooseControlNew)

        methods = [f'{MOOSECONTROL}.initialize', f'{MOOSECONTROL}.finalize']
        self.assertMethodsCalledInOrder(methods, action)

class TestMooseControlSetUpControl(MooseControlTestCase):
    """
    Tests moosecontrol.moosecontrol.MooseControl, automatically
    setting up a MooseControlNew for each case.
    """
    def setUp(self):
        # The control for this test
        self.control: MooseControlNew = MooseControlNew(BaseRunnerTest())

        # Paths in order that were called using mocked Session.get
        self.get_paths: list[str] = []
        # Path and JSON data that were called using mocked Session.post
        self.post_paths: list[Tuple[str, dict]] = []

        # Setup the session for the control (would be done in initialize())
        self.control.runner._session = self.control.runner.build_session()

        super().setUp()

    def tearDown(self):
        # Clean up the session
        self.control.runner._session.close()

        super().tearDown()

    def mock_get(self,
                 paths: list[Tuple[str, dict]] = [],
                 waiting: Optional[bool] = None):
        """
        Mocks Session.get for the runner.
        """
        self.get_paths.clear()

        paths = deepcopy(paths)
        runner = self.control.runner

        if waiting is not None:
            if waiting:
                paths.append((
                    'waiting',
                    {'data': {'waiting': True, 'execute_on_flag': FAKE_EXECUTE_ON_FLAG}}
                ))
            else:
                paths.append((
                    'waiting',
                    {'data': {'waiting': False}}
                ))

        orig = runner._session.get
        def new(path):
            self.get_paths.append(path.replace(f'{FAKE_URL}/', ''))
            for mock_path, mock_kwargs in paths:
                if path.endswith(f'/{mock_path}'):
                    return mock_response(url=path, **mock_kwargs)
            return orig(path)

        return patch.object(runner._session, 'get', new=new)

    def assertGetPaths(self, paths: list[str]):
        """
        Asserts that the given paths were passed in the mocked get.
        """
        self.assertEqual(self.get_paths, paths)

    def mock_post(self,
                  paths: list[Tuple[str, dict]] = []):
        """
        Mocks session.post for the runner.
        """
        self.post_paths.clear()

        paths = deepcopy(paths)
        runner = self.control.runner

        orig = runner._session.post
        def new(path, json):
            self.post_paths.append((
                path.replace(f'{FAKE_URL}/', ''),
                json
            ))
            for mock_path, mock_kwargs in paths:
                if path.endswith(f'/{mock_path}'):
                    return mock_response(url=path, **mock_kwargs)
            return orig(path)

        return patch.object(runner._session, 'post', new=new)

    def assertPostPaths(self, paths: list[Tuple[str, dict]]):
        """
        Asserts that the given paths and data were passed in the mocked get.
        """
        self.assertEqual(self.post_paths, paths)

    def test_get_waiting_flag(self):
        """
        Tests get_waiting_flag().
        """
        with self.mock_get(waiting=False) as get:
            self.assertIsNone(self.control.get_waiting_flag())
        self.assertGetPaths(['waiting'])

        with self.mock_get(waiting=True):
            self.assertEqual(self.control.get_waiting_flag(), FAKE_EXECUTE_ON_FLAG)
        self.assertGetPaths(['waiting'])

    def test_is_waiting(self):
        """
        Tests is_waiting().
        """
        with self.mock_get(waiting=False):
            self.assertFalse(self.control.is_waiting())
        self.assertGetPaths(['waiting'])

        with self.mock_get(waiting=True):
            self.assertTrue(self.control.is_waiting())
        self.assertGetPaths(['waiting'])

    def test_require_waiting_not_waiting(self):
        """
        Tests require_waiting() when the control is not waiting.
        """
        with self.mock_get(waiting=False):
            with self.assertRaises(ControlNotWaiting):
                self.control.require_waiting()
        self.assertGetPaths(['waiting'])

    def test_require_waiting(self):
        """
        Tests require_waiting().
        """
        with self.mock_get(waiting=True):
            self.control.require_waiting()
        self.assertGetPaths(['waiting'])

    def test_set_continue(self):
        """
        Tests set_continue().
        """
        with self.mock_get(waiting=True, paths=[('continue', {})]):
            self.control.set_continue()
        self.assertGetPaths(['waiting', 'continue'])
        self.assertLogSize(1)
        self.assertLogMessage(0, 'Sending continue to webserver')

    def test_set_terminate(self):
        """
        Tests set_terminate().
        """
        with self.mock_get(waiting=True, paths=[('terminate', {})]) as thing:
            self.control.set_terminate()
        self.assertEqual(self.get_paths, ['waiting', 'terminate'])
        self.assertLogSize(1)
        self.assertLogMessage(0, 'Sending terminate to webserver')

    def test_get_postprocessor(self):
        """
        Tests get_postprocessor().
        """
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            with self.mock_get(waiting=True):
                paths = [('get/postprocessor', {'data': {'value': value_type(value)}})]
                with self.mock_post(paths):
                    get_value = self.control.get_postprocessor(FAKE_NAME)
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(['waiting'])
            self.assertPostPaths([('get/postprocessor', {'name': FAKE_NAME})])
            self.assertLogSize(1)
            self.assertLogMessage(
                0,
                f'Getting postprocessor value for "{FAKE_NAME}"',
                levelname='DEBUG'
            )

    def test_get_reporter_value(self):
        """
        Tests get_reporter_value().
        """
        value = 100
        with self.mock_get(waiting=True):
            paths = [('get/reporter', {'data': {'value': value}})]
            with self.mock_post(paths):
                get_value = self.control.get_reporter_value(FAKE_NAME)

        self.assertEqual(value, get_value)
        self.assertGetPaths(['waiting'])
        self.assertPostPaths([('get/reporter', {'name': FAKE_NAME})])
        self.assertLogSize(1)
        self.assertLogMessage(
            0,
            f'Getting reporter value for "{FAKE_NAME}"',
            levelname='DEBUG'
        )

    def test_get_time(self):
        """
        Tests get_time().
        """
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            paths = [('get/time', {'data': {'time': value_type(value)}})]
            with self.mock_get(waiting=True, paths=paths):
                get_value = self.control.get_time()
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(['waiting', 'get/time'])
            self.assertLogSize(1)
            self.assertLogMessage(
                0,
                'Getting simulation time',
                levelname='DEBUG'
            )

    def test_get_dt(self):
        """
        Tests get_dt().
        """
        value = 5.0

        for value_type in [float, int]:
            self._caplog.clear()
            paths = [('get/dt', {'data': {'dt': value_type(value)}})]
            with self.mock_get(waiting=True, paths=paths):
                get_value = self.control.get_dt()
            self.assertIsInstance(get_value, float)
            self.assertEqual(value, get_value)
            self.assertGetPaths(['waiting', 'get/dt'])
            self.assertLogSize(1)
            self.assertLogMessage(
                0,
                'Getting simulation timestep',
                levelname='DEBUG'
            )

    def test_set_controllable_scalar_bad_type(self):
        """
        Tests _set_controllable_scalar() with a bad type.
        """
        with self.assertRaises(TypeError) as e:
            self.control._set_controllable_scalar(
                'unused',
                'unused',
                (Number,),
                'foo'
            )
        self.assertEqual(
            str(e.exception),
            'Type str is not of allowed type(s) Number'
        )

    def test_set_controllable_scalar_convert_type(self):
        """
        Tests _set_controllable_scalar() converting input values
        into a common type.
        """
        value = 1

        # Converts int -> float
        with self.mock_get(waiting=True):
            with self.mock_post([('set/controllable', {'status_code': 201})]):
                self.control._set_controllable_scalar(
                    FAKE_NAME,
                    'Real',
                    (Number,),
                    value,
                    float
                )
        post_value = self.post_paths[0][1]['value']
        self.assertIsInstance(post_value, float)

    def run_test_set_controllable(self, value: Any, cpp_type: str, method_type: str):
        """
        Helper for testing the various set_controllable_XXX() routines.
        """
        with self.mock_get(waiting=True):
            with self.mock_post([('set/controllable', {'status_code': 201})]):
                method = getattr(self.control, f'set_controllable_{method_type}')
                method(FAKE_NAME, value)
        self.assertGetPaths(['waiting'])
        self.assertPostPaths([(
            'set/controllable',
            {'name': FAKE_NAME, 'value': value, 'type': cpp_type}
        )])
        self.assertLogSize(1)
        self.assertLogMessage(
            0,
            f'Setting controllable value "{FAKE_NAME}"',
            levelname='DEBUG'
        )

    def test_set_controllable_bool(self):
        """
        Tests set_controllable_bool().
        """
        self.run_test_set_controllable(True, 'bool', 'bool')

    def test_set_controllable_int(self):
        """
        Tests set_controllable_int().
        """
        self.run_test_set_controllable(int(1), 'int', 'int')

    def test_set_controllable_real(self):
        """
        Tests set_controllable_real().
        """
        self.run_test_set_controllable(float(1), 'Real', 'real')

    def test_set_controllable_string(self):
        """
        Tests set_controllable_string().
        """
        self.run_test_set_controllable('foo', 'std::string', 'string')

    def test_set_controllable_vector_bad_type(self):
        """
        Tests _set_controllable_vector() with a bad type.
        """
        with self.assertRaises(TypeError) as e:
            self.control._set_controllable_vector(
                'unused',
                'unused',
                (Number,),
                ['foo'],
            )
        self.assertEqual(
            str(e.exception),
            'At index 0: type list is not of allowed type(s) Number'
        )

    def test_set_controllable_vector_convert_type(self):
        """
        Tests _set_controllable_vector() converting input values
        into a common type.
        """
        value = [1, 2, 3]
        self.assertFalse(all(isinstance(v, float) for v in value))
        float_value = [float(v) for v in value]

        # Converts list[int] -> list[float]
        with self.mock_get(waiting=True):
            with self.mock_post([('set/controllable', {'status_code': 201})]):
                self.control._set_controllable_vector(
                    FAKE_NAME,
                    'Real',
                    (Number,),
                    value,
                    float
                )
        post_value = self.post_paths[0][1]['value']
        self.assertTrue(all(isinstance(v, float) for v in post_value))

    def test_set_controllable_vector_int(self):
        """
        Tests set_controllable_vector_int().
        """
        self.run_test_set_controllable([1, 2, 3], 'std::vector<int>', 'vector_int')

    def test_set_controllable_vector_real(self):
        """
        Tests set_controllable_vector_int().
        """
        value = [1.0, 2, 3.0]
        self.assertFalse(all(isinstance(v, float) for v in value))

        self.run_test_set_controllable(value, 'std::vector<Real>', 'vector_real')

        # Converted all to float
        post_value = self.post_paths[0][1]['value']
        self.assertTrue(all(isinstance(v, float) for v in post_value))

    def test_set_controllable_vector_real(self):
        """
        Tests set_controllable_vector_int().
        """
        value = [1.0, 2, 3.0]
        self.assertFalse(all(isinstance(v, float) for v in value))

        self.run_test_set_controllable(value, 'std::vector<Real>', 'vector_real')

        # Converted all to float
        post_value = self.post_paths[0][1]['value']
        self.assertTrue(all(isinstance(v, float) for v in post_value))

    def test_set_controllable_vector_string(self):
        """
        Tests set_controllable_vector_string().
        """
        self.run_test_set_controllable(
            ['foo', 'bar'],
            'std::vector<std::string>',
            'vector_string'
        )
