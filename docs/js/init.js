(function($){
  $(function(){

    $('.button-collapse').sideNav();
    $('.parallax').parallax();
    $('h2').addClass("section scrollspy");
    $('.toc-wrapper').pushpin({
        top: 152
    });
    $('.modal').modal();
    $('.scrollspy').scrollSpy();
      $('.slider').slider();

    $('table.moose-parameter-table-outer').addClass('bordered striped');
    $('table.moose-subobjects-table').addClass('bordered striped');
    $('table.moose-subsystems-table').addClass('bordered striped');

  }); // end of document ready
})(jQuery); // end of jQuery name space

// Setup MathJax
MathJax.Hub.Config({
    tex2jax: {
        skipTags: ['script', 'noscript', 'style', 'textarea', 'pre'],
        inlineMath: [['$','$'], ['\\(','\\)']]
    }
});

// The prism package for highlighting code requires the class
// "language-*" be defined in the <pre> tag, which the fenced
// code blocks of python markdown do not do, this little script
// adds this class.
function prism() {
   var code = document.getElementsByTagName("CODE");
   for (var i = 0; i < code.length; ++i)
   {
     var parent = code[i].parentNode
     if (parent.nodeName == 'PRE')
     {
     	parent.className = "language-" + code[i].className;
     }
   }
}
prism();
