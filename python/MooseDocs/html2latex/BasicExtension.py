from Extension import Extension
import elements

class BasicExtension(Extension):
    """
    Basic set of tag Element conversion objects.
    """

    def extend(self, translator):
        """
        Add the basic Element objects to the Translator instance.
        """
        super(BasicExtension, self).extend(translator)

        # Headings
        config = self.getConfigs()
        headings = config['headings']
        if headings and len(headings) != 6:
            raise Exception("Invalid headings list, you must supply a list of 6 valid latex commands.")
        for i, h in enumerate(headings):
            name = 'h{}'.format(i+1)
            obj = eval("elements.h{}(command='{}')".format(i+1, h))
            translator.elements.add(name, obj)

        translator.elements.add('div', elements.div())
        translator.elements.add('pre_code', elements.pre_code())
        translator.elements.add('pre', elements.pre())
        translator.elements.add('ol', elements.ol())
        translator.elements.add('ul', elements.ul())
        translator.elements.add('hr', elements.hr())
        translator.elements.add('inline_equation', elements.inline_equation())
        translator.elements.add('equation', elements.equation())
        translator.elements.add('table', elements.table())
        translator.elements.add('figure', elements.figure())
        translator.elements.add('img', elements.img())

        # Inline Elements
        translator.elements.add('thead', elements.thead())
        translator.elements.add('tbody', elements.tbody())
        translator.elements.add('tr', elements.tr())
        translator.elements.add('a', elements.a())
        translator.elements.add('p', elements.p())
        translator.elements.add('span', elements.span())
        translator.elements.add('li', elements.li())
        translator.elements.add('em', elements.em())
        translator.elements.add('code', elements.code())
        translator.elements.add('figcaption', elements.figcaption())
