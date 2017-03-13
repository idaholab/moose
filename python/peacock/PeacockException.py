
class PeacockException(Exception):
    pass

class FileExistsException(PeacockException):
    pass

class BadExecutableException(PeacockException):
    pass

