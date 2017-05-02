# Flow Charts
The ability to include diagrams using [GraphViz](http://www.graphviz.org/) using the [dot]() language is provided.
Simply, include the "dot" syntax in the markdown, being sure to include the keywords ("graph" or
"digraph") on the start of a new line.

* The official page for the dot language is detailed here: [dot](http://www.graphviz.org/content/dot-language)
* There are many sites that provide examples, for example:
    * [https://en.wikipedia.org/wiki/DOT_(graph_description_language)](https://en.wikipedia.org/wiki/DOT_(graph_description_language))
    * [http://graphs.grevian.org/example](http://graphs.grevian.org/example)
* There also exists live, online tools for writing dot:
    * [http://dreampuf.github.io/GraphvizOnline/](http://dreampuf.github.io/GraphvizOnline/)
    * [http://www.webgraphviz.com/](http://www.webgraphviz.com/)

For example, the following dot syntax placed directly in the markdown produces the following graph.
```text
graph {
    bgcolor="#ffffff00" // transparent background
    a -- b -- c;
    b -- d;
}
```

graph {
    bgcolor="#ffffff00" // transparent background
    a -- b -- c;
    b -- d;
}

!extension DiagramExtension title=Diagram Extension Configuration Options

!extension-settings moose_diagrams title=graph|digraph Command Settings
