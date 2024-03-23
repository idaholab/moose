import os, pathlib, sys
from MooseControl import MooseControl

def base_controller(control_name, run_control):
    # Get the command we should run
    RUNAPP_COMMAND = os.environ.get('RUNAPP_COMMAND')
    if RUNAPP_COMMAND is None:
        sys.exit('Missing expected command variable RUNAPP_COMMAND')

    # Find a port to run on and add it to the command
    port = MooseControl.getAvailablePort()
    command = f'{RUNAPP_COMMAND} Controls/{control_name}/port={port}'

    # Start up moose
    print(f'Spawning MOOSE with command "{command}"')
    cwd = pathlib.Path(__file__).parent.resolve()
    process = MooseControl.spawnMoose(command, cwd)

    # Build the MooseControl for the test to use
    control = MooseControl(port)
    control.initialWait()

    # Run the user test, killing the process on any exceptions
    try:
        run_control(control)
    except:
        process.kill()
        raise

    # Wait for MOOSE to finish up
    control.waitDone()
    process.wait()

    # Append the MOOSE output
    output = ''.join(process.stdout.readlines())
    print(output)
    if process.returncode != 0:
        sys.exit(process.returncode)

def expect_equal(gold, value):
    if gold != value:
        raise Exception(f'"{gold}" != "{value}"')
