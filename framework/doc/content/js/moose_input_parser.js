// Function for gathering data
function getSyntaxData(element) {
    // Use regex to extract the text we want to display and meta data for the element
    const content = element.textContent;
    console.log(content);
    const match = /(\w+)\<\<\<(.*?)\>\>\>/.exec(content);
    if (!match || match.length < 3) {
        return element;
    }
    const text = match[1];
    const data = JSON.parse(match[2]);

    // Change the element to an 'a' and add a reference to the page
    let new_element;
    if ('href' in data) {
        new_element = document.createElement('a');
        new_element.className = element.className;
        new_element.href = data.href;
    } else {
        new_element = element;
    }

    // Add a tooltip title for the object description
    if ('description' in data) {
        new_element.setAttribute('title', data.description);
    }

    // Replace the text so it doesn't have the meta-data
    new_element.textContent = text;
    return new_element;
}

window.addEventListener('load', function () {
    // All the classes with meta data captured by prism
    const parsed_classes = ["moose-parsed-syntax", "moose-parsed-class", "moose-parsed-parameter"];

    // Get all the elements with the prism-specific classes
    const elementsArray = parsed_classes.flatMap(parsed_class => 
        Array.from(document.getElementsByClassName(parsed_class))
    );

    // Build the new elements by extracting the meta data
    const newElementsArray = elementsArray.map(getSyntaxData);

    // Relace the elements with the new ones
    elementsArray.forEach((element, index) => {
        element.replaceWith(newElementsArray[index]);
    });
});
