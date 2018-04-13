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

var options = {
  shouldSort: true,
  threshold: 0.5,
  minMatchCharLength: 2,
  keys: [
    {
      name: "name",
      weight: 0.75
    },
    {
      name: "text",
      weight: 0.5
    }
  ]
};

function mooseSearch() {
    var element = document.getElementById("moose-search-results");
    var box = document.getElementById("moose-search-box");

    var fuse = new Fuse(index_data, options);
    var results = fuse.search(box.value);

    console.log(results.length);
    if (results.length > 0)
    {
        element.innerHTML = '';
        for (var item of results)
        {
            var div = document.createElement("div");
            div.className = 'moose-search-result';
            element.appendChild(div);

            var title = document.createElement("div");
            title.className = 'moose-search-result-title';
            var a = document.createElement("a");
            a.innerHTML = item.name;
            a.setAttribute("href", item.location);
            title.appendChild(a);
            div.appendChild(title);
        }
    }
    else
    {
        element.innerHTML = '';
    }
};
