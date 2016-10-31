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
  }); // end of document ready
})(jQuery); // end of jQuery name space

// Setup MathJax
MathJax.Hub.Config({
    tex2jax: {
        skipTags: ['script', 'noscript', 'style', 'textarea', 'pre'],
        inlineMath: [['$','$'], ['\\(','\\)']]
    }
});
