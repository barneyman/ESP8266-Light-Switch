//let base = "http://192.168.42.18"
let base = "."


function headerLoaded() {

    let url = base + '/json/config'

    var iframe = window.frames['header']

    fetch(url)
        .then(function (response) { return response.json(); })
        .then(function (data) {

            // get the name
            iframe.document.getElementById('name').innerText = data['name']
        })



}
