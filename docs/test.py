import bs4
import MooseDocs

html = r'<pre><code>foo</code></pre>'
soup = bs4.BeautifulSoup(html, "html.parser")

for tag in soup.descendants:
    print tag.name, tag.parent.name
