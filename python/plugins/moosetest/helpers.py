import os

def get_file_base(moose_object):
    """
    """
    if '_hit_filename' not in moose_object.parameters():
        base_dir = os.getcwd()
        msg = "The `MooseAppRunner` with name '{}' was not created via a HIT file, as such the base directory for files/paths needed to execute the object are not known. The current working directory of '{}' is being used."
        self.warning(msg, moose_object.name(), base_dir)
    else:
        hit_file = moose_object.getParam('_hit_filename')
        base_dir = os.path.dirname(hit_file)

    return base_dir
