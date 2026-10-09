// Initializes the revolving image galleries created by the MooseDocs gallery extension's
// 'slideshow' command. Each slideshow is a Materialize carousel in full-width (one slide at
// a time) mode. Materialize supplies the slide transitions and the indicator dots, but not
// autoplay, so that is added here from the token's data attributes.
(function($){
    $(function(){
        $('.carousel.moose-slideshow').each(function(){
            var carousel = this;
            var $carousel = $(carousel);

            // jQuery's data() coerces numeric strings to numbers.
            var interval = parseInt($carousel.data('interval'), 10);

            var instance = M.Carousel.init(carousel, {fullWidth : true, indicators : true});

            // Materialize sizes the carousel to the active slide's image, which clips anything
            // beneath the image and any slide taller than the first. Size it to the tallest
            // slide instead, plus a strip beneath for the indicator dots (8px dots with 24px
            // vertical margins). Replacing the private _setCarouselHeight method (Materialize is
            // vendored, so its version is fixed) also covers the calls Materialize makes on
            // window resize. The image load handlers run after the one Materialize registered
            // during init, so they win over its image-height setting.
            var setHeight = function() {
              var height = 0;
              $carousel.find('.carousel-item')
                  .each(function() { height = Math.max(height, $(this).outerHeight()); });
              $carousel.css('height', (height + 56) + 'px');
            };
            instance._setCarouselHeight = setHeight;
            $carousel.find('img').on('load', setHeight);
            setHeight();

            // Never lay out a hidden (zero-width) slideshow, such as one in an inactive tab:
            // with zero-width slides Materialize computes the current slide as 0/0, and the
            // resulting NaN stops the slideshow for good. The layout from when it was last
            // visible stays valid while it is hidden. This guards Materialize's own window
            // resize handler, which calls _handleResize.
            var handleResize = instance._handleResize.bind(instance);
            instance._handleResize = function() {
              if (carousel.offsetWidth > 0)
              {
                handleResize();
                instance._scroll(); // moves the slides to the new positions
              }
            };

            // Materialize measures the slide width only at init and on window resize, so a
            // slideshow that starts hidden is laid out with zero-width slides that stack on top
            // of each other. Redo the layout whenever the carousel's width changes, which
            // includes it becoming visible. The width check skips the height changes that
            // setHeight itself causes.
            var width = carousel.offsetWidth;
            new ResizeObserver(function() {
              if (carousel.offsetWidth !== width)
              {
                width = carousel.offsetWidth;
                instance._handleResize(); // recomputes slide width and calls setHeight
              }
            }).observe(carousel);

            if (!isNaN(interval) && interval > 0) {
                // Pause autoplay while a slide image's Materialbox lightbox is open:
                // advancing the carousel underneath the zoomed image transforms the very
                // image being viewed, which looks buggy. Materialbox inserts a single
                // '#materialbox-overlay' element for the whole open-and-closing window, so
                // its presence is a reliable "a lightbox is open" signal.
                var timer = setInterval(function() {
                  if (!document.getElementById('materialbox-overlay'))
                  {
                    instance.next();
                  }
                }, interval);

                // Clicking an indicator dot means the viewer has taken control of the
                // slideshow, so stop autoplay for good rather than advance past their choice.
                // Bind on the dots themselves (created by M.Carousel.init above): the click
                // does not bubble up to the carousel element.
                $carousel.find('.indicator-item').on('click', function() { clearInterval(timer); });
            }
        });
    });
})(jQuery);
