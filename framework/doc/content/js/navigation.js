(function($){
    $(function(){

        $('h2').addClass("section scrollspy");
        $('.toc-wrapper').pushpin({
            top: 162
        });
        $('.scrollspy').scrollSpy();

        $('.sidenav').sidenav();

        $('.moose-mega-menu-trigger, .moose-mega-menu-content').hover(
            function(){
                inMooseMegaMenuTrigger(this);
            },
            function(){
                outMooseMegaMenuTrigger(this);
            }
        );
    }); // end of document ready
})(jQuery); // end of jQuery name space

function getMooseMegaMenu(element)
{
    var menu;
    if (element.classList.contains('moose-mega-menu-content')){
        menu = element;
    }
    else{
        var id = element.getAttribute('data-target')
        var menu = document.getElementById(id)
    }
    return menu;
}

function inMooseMegaMenuTrigger(element)
{
    var menu = getMooseMegaMenu(element);
    menu.style.display = 'block'
}

function outMooseMegaMenuTrigger(element)
{
    var menu = getMooseMegaMenu(element);
    menu.style.display = 'none'
}


var options = {
  shouldSort: true,
  threshold: 0.3,
  minMatchCharLength: 3,
  keys: [
    {
      name: "text",
      weight: 1
    }
  ]
};

function mooseSearch() {
    var element = document.getElementById("moose-search-results");
    var box = document.getElementById("moose-search-box");

    var home = document.getElementById("home-button").href;
    const homedir = home.substr(0, home.lastIndexOf("/"));

    var fuse = new Fuse(index_data, options);
    var results = fuse.search(box.value);

    console.log(results);
    var n = results.length;
    if (n > 0 && box.value.length > 2)
    {
        element.innerHTML = '';
        for (var i = 0; i < n && i < 30; ++i)
        {
            var item = results[i];
            var div = document.createElement("div");
            div.className = 'moose-search-result';

            if (item.title)
            {
                var title = document.createElement("div");
                title.className = 'moose-search-result-title';
                var a = document.createElement("a");
                a.innerHTML = item.title;
                a.setAttribute("href", homedir + '/' + item.location);
                title.appendChild(a);
            }

            if (item.title != item.text) {
              var section = document.createElement("span");
              section.innerHTML = ' &mdash; ' + item.text;
              a.appendChild(section);
            } else {
              a.setAttribute("style", "font-weight: bold");
            }

            div.appendChild(title);
            element.appendChild(div);
        }
    }
    else
    {
        element.innerHTML = '';
    }
};
