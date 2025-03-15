// Function for gathering data
function getSyntaxData(element) {
    // Use regex to extract the text we want to display and meta data for the element
    content = element.innerHTML
    const match = /(\w+)(?:&lt;&lt;&lt;)(.*?)(?=&gt;&gt;&gt)/.exec(content);
    if (!match || match.length < 3)
        return element
    const text = match[1];
    const data = JSON.parse(match[2]);

    // Change the element to an 'a' and add a reference to the page
    let new_element;
    if ('href' in data){
        new_element = document.createElement('a');
        new_element.className = element.className;
        new_element.href = data.href;
    }
    else {
        new_element = element;
    }

    // Add a tooltip title for the object description
    if ('description' in data) {
        new_element.setAttribute('title', data.description)
    }

    // Replace the text so it doesn't have the meta-data
    new_element.innerHTML = text;
    return new_element
}

window.addEventListener('load', function () {
    // All the classes with meta data captured by prism
    const parsed_classes = ["moose-parsed-syntax", "moose-parsed-class", "moose-parsed-parameter"];

    // Loop through all of these parsed classes and insert meta-data
    for (const i in parsed_classes) {
        let elements = document.getElementsByClassName(parsed_classes[i]);
        for (let elem of elements) {
            // The new element created from the meta-data
            let new_element = getSyntaxData(elem);
            // Replace this element with the new one since the tag may have changed
            elem.parentElement.replaceChild(new_element, elem)
        }
    }
})
