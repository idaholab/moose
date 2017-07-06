(function($){
  $(function(){

   // $('h2').addClass("section scrollspy");
    $('.toc-wrapper').pushpin({
        top: 162
    });
    $('.modal').modal();
    $('.scrollspy').scrollSpy();
    $('.slider').slider();

    $('.dropdown-button').dropdown({
      inDuration: 300,
      outDuration: 225,
      constrain_width: false, // Does not change width of dropdown to that of the activator
      hover: true, // Activate on hover
      gutter: 0, // Spacing from edge
      belowOrigin: true, // Displays dropdown below the button
      alignment: 'left' // Displays dropdown with edge aligned to the left of button
    });
  }); // end of document ready
})(jQuery); // end of jQuery name space

// Setup MathJax
MathJax.Hub.Config({
  config: ["MMLorHTML.js"],
  jax: ["input/TeX", "output/HTML-CSS", "output/NativeMML"],
  extensions: ["MathMenu.js", "MathZoom.js"],
  TeX: { equationNumbers: { autoNumber: "AMS" } }
});

// Copy code button
var clipboard = new Clipboard('.moose-copy-button');

// Function for latex equation references
MathJax.Hub.Queue(function(){
  $('.moose-equation-reference').each(function(i, e) {
    var elem = $($(e).attr('href'));
    if (elem.length) {
      var txt = $('.mtext', elem).text()
      $(e).text(txt);
      console.log('Located reference to Equation ' + txt);
    } else {
      console.error('Unable to located reference to equation: ' + $(e).attr('href'));
      $(e).text('(??)');
    }
  });
});
