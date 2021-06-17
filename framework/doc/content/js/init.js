(function($){
    $(function(){
        $('.tabs').tabs();
        $('.tooltipped').tooltip({enterDelay:1000});
        $('.modal').modal();
        $('.collapsible').collapsible();
        $('.slider').slider();
        $('.materialboxed').materialbox();
        $('.dropdown-trigger').dropdown({
            inDuration: 300,
            outDuration: 225,
            constrainWidth: false, // Does not change width of dropdown to that of the activator
            hover: true, // Activate on hover
            coverTrigger: false, // Displays dropdown below the button
            alignment: 'left' // Displays dropdown with edge aligned to the left of button
        });

        // Set the height of example code/html to be the same
        $('.moose-devel-example').each(function(){
            var div_code = $(this).find('.moose-devel-example-code')
            var div_html = $(this).find('.moose-devel-example-html')
            h = Math.max(div_code.height(), div_html.height());
            div_html.height(h);
            div_code.height(h);
        });
    }); // end of document ready
})(jQuery); // end of jQuery name space
