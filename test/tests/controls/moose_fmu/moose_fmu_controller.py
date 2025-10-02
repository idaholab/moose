import os, sys

# Helper for testing the MOOSE FMU via the RunApp command_proxy option
def test_controller():
    # Get the command we should run
    # You'll hit this if you don't run a RunApp-derived Tester or
    # don't run it with the "command_proxy" option
    RUNAPP_COMMAND = os.environ.get('RUNAPP_COMMAND')
    if RUNAPP_COMMAND is None:
        sys.exit('Missing expected command variable RUNAPP_COMMAND')

    return RUNAPP_COMMAND
