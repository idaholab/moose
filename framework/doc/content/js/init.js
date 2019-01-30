(function($){
    $(function(){

        $('.modal').modal();
        $('.slider').slider();
        $('.dropdown-trigger').dropdown({
            inDuration: 300,
            outDuration: 225,
            constrainWidth: false, // Does not change width of dropdown to that of the activator
            hover: true, // Activate on hover
            coverTrigger: false, // Displays dropdown below the button
            alignment: 'left' // Displays dropdown with edge aligned to the left of button
        });

   // $('.collapsible').collapsible({
   //   onOpen: function(el) {var icons = $(el).find('i'); $(icons[0]).text('keyboard_arrow_up');},
   //   onClose: function(el) {var icons = $(el).find('i'); $(icons[0]).text('keyboard_arrow_down');}
      // });

  }); // end of document ready
})(jQuery); // end of jQuery name space
