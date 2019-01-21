// when testing these web pages from the PC ...
// chrome --disable-web-security --user-data-dir="[some directory here]"
//let base = "http://192.168.82.135"
let base = "."


function headerLoaded() {

    let url = base + '/json/config'

    var iframe = window.frames['header']

    fetch(url)
        .then(function (response) { return response.json(); })
        .then(function (data) {

            // get the name
            iframe.document.getElementById('name').innerText = data['name']
            iframe.document.getElementById('version').innerText = data['version']
            iframe.document.getElementById('versionHTML').innerText = data['versionHTML']
            
        })



}
