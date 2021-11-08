(function($){
    $(function(){

        $('.moose-sqa-badges a.modal-trigger').click(
            function(){
                onGetCivetResults(this);
            }
        );

    }); // end of document ready
})(jQuery); // end of jQuery name space

function onGetCivetResults(element)
{
    var id = element.getAttribute('href')
    var content = document.getElementById(id.substring(1)).firstChild;
    var data = content.getAttribute('data-civet-results');
    //var obj = JSON.parse(data);

    console.log(data);

    var obj = JSON.parse(data);
    //for (var key in data)
    //
    //   var title = document.createElement('span');
    //   title.appendChild(document.createTextNode(key));
    //   content.appendChild(title);
    //   //var value = data[key]

    //
}
