import tools

def test():
    tools.executeAppAndDiff(__file__,'restart_diffusion_test_steady.i',['steady_out.e'])
    tools.executeAppAndDiff(__file__,'restart_diffusion_test_transient.i',['out.e'])
