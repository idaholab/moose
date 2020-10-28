(function($){
    $(function(){
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
    }); // end of document ready

    // Change video source for dark mode, see media.py RenderVideo
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
        $('.moose-video source[data-dark-src]').each(function(){
            var src = $(this).data('dark-src');
            $(this).attr('src', src);
        });
    };
})(jQuery); // end of jQuery name space
