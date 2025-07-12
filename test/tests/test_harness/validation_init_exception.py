from TestHarness.validation import ValidationCase

class TestCase(ValidationCase):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        raise Exception('foo')
