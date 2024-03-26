import os, pathlib, sys
from MooseControl import MooseControl

# Helper for testing the WebServerControl via the RunApp command_proxy option
def base_controller(control_name, run_control):
    # Get the command we should run
    # You'll hit this if you don't run a RunApp-derived Tester or
    # don't run it with the "command_proxy" option
    RUNAPP_COMMAND = os.environ.get('RUNAPP_COMMAND')
    if RUNAPP_COMMAND is None:
        sys.exit('Missing expected command variable RUNAPP_COMMAND')

    # Build the MooseControl for the test to use
    control = MooseControl(moose_command=RUNAPP_COMMAND, moose_control_name=control_name)
    control.initialize()

    # Run the user test
    run_control(control)

    # Wait for the webserver to stop listening
    control.finalize()

def expect_equal(gold, value):
    if gold != value:
        raise Exception(f'"{gold}" != "{value}"')
