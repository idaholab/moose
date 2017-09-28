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

    $('.collapsible').collapsible({
      onOpen: function(el) {var icons = $(el).find('i'); $(icons[0]).text('keyboard_arrow_up');},
      onClose: function(el) {var icons = $(el).find('i'); $(icons[0]).text('keyboard_arrow_down');}
    });

    $(".button-collapse").sideNav();
  }); // end of document ready
})(jQuery); // end of jQuery name space

// Copy code button
var clipboard = new Clipboard('.moose-copy-button');

// Function for latex equation references
$(document).ready(function(){
  $('.moose-equation-reference').each(function(i, e) {
    var elem = $($(e).attr('href'));
    if (elem.length) {
      var txt = elem.data('moose-katex-equation-number')
      $(e).text(txt);
      console.log('Located reference to Equation ' + txt);
    } else {
      console.error('Unable to located reference to equation: ' + $(e).attr('href'));
      $(e).text('(??)');
    }
  });
});

$(document).ready(function(){
  var elems = document.getElementsByClassName("moose-page-status")
  for (var i = 0; i < elems.length; i++)
  {
    var url = $(elems[i]).data("filename")
    $(elems[i]).load(url + " #moose-status")
  }
});
