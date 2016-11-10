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
  config: ["MMLorHTML.js"],
  jax: ["input/TeX", "output/HTML-CSS", "output/NativeMML"],
  extensions: ["MathMenu.js", "MathZoom.js"]
});
