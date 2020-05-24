// when testing these web pages from the PC ... base should point to the esp mule
// chrome --disable-web-security --user-data-dir="[some directory here]"
//let base = "http://192.168.42.117"
let base = "."

function headerLoaded(){let url=base+"/json/config";var iframe=window.frames.header;fetch(url).then((function(response){return response.json()})).then((function(data){iframe.document.getElementById("name").innerText=data.name;const verData=data.version.split("|");iframe.document.getElementById("hardware").innerText=verData[0],iframe.document.getElementById("version").innerText=verData[1]}))}function showStatus(text){document.getElementById("statustext").innerText=text,document.getElementById("statusbar").hidden=!1}function clearStatus(){document.getElementById("statustext").innerText="",document.getElementById("statusbar").hidden=!0}