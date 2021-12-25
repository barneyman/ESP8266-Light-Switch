// when testing these web pages from the PC ... base should point to the esp mule
// chrome --disable-web-security --user-data-dir="[some directory here]"
let base = "http://192.168.4.1"
    //let base = "."

function headerLoaded() {
    let url = base + "/json/config";
    var iframe = window.frames.header;
    fetch(url).then((function(response) {
        return response.json()
    })).then((function(data) {
        iframe.document.getElementById("name").innerText = data.name;
        const verData = data.version.split("|");
        iframe.document.getElementById("hardware").innerText = verData[0],
            iframe.document.getElementById("version").innerText = verData[1]
    }))
}

function showStatus(text) {
    document.getElementById("statustext").innerText = text,
        document.getElementById("statusbar").hidden = !1
}

function clearStatus() {
    document.getElementById("statustext").innerText = "",
        document.getElementById("statusbar").hidden = !0
}

function addConfigItems(container, name, configItems, hideit = true) {
    var configFieldSet = document.createElement("fieldset")
    var configLegend = document.createElement("legend")
    configLegend.innerText = name;
    configFieldSet.appendChild(configLegend)


    configItems.forEach(function(item) {

        var configDiv = document.createElement("div")
        configDiv.className = "horizontalContainer"

        configFieldSet.appendChild(configDiv)

        var configName = document.createElement("div")
        configName.innerText = item["name"]

        configDiv.appendChild(configName)

        switch (item["type"]) {
            case "select":
                var input = document.createElement("select")

                for (var eachOpt in item["options"]) {
                    if (item["options"].hasOwnProperty(eachOpt)) {
                        var opt = document.createElement("option")
                        opt.value = item["options"][eachOpt]["value"]
                        opt.innerText = item["options"][eachOpt]["name"]

                        input.appendChild(opt)

                    }
                }

                configDiv.appendChild(input)
                break;
            case "text":
                var input = document.createElement("input")
                input.type = "text"
                input.name = item["name"]
                if (item.hasOwnProperty("default")) {
                    input.value = item["default"]
                }
                configDiv.appendChild(input)
                break
            case "number":
                var input = document.createElement("input")
                input.type = "number"
                input.name = item["name"]
                if (item.hasOwnProperty("default")) {
                    input.value = item["default"]
                }
                configDiv.appendChild(input)
                break

        }


    });

    configFieldSet.hidden = hideit

    container.appendChild(configFieldSet)


}


// device config stuff - used on AP when onboarding, and STA probbaly never
// needs 
//
// deviceConfigDiv
// additionaDevicesSelect
// deviceInstalledDiv
// 

function getDeviceConfigsFieldset() {
    var configDiv = document.getElementById("deviceConfigDiv")
    var configs = configDiv.getElementsByTagName("fieldset")
    return configs
}


function showCorrectDeviceConfig() {
    // get the impl that's selected, and unhide that tab
    var configs = getDeviceConfigsFieldset()
    var selector = document.getElementById("additionaDevicesSelect");
    var selected = selector.options[selector.selectedIndex].innerText

    for (var loop = 0; loop < configs.length; loop++) {
        item = configs.item(loop)

        if (item.getElementsByTagName("legend")[0].innerText == selected)
            item.hidden = false
        else
            item.hidden = true
    }
}

function populateConfiguredDevices() {
    let url = base + '/json/devices'

    fetch(url)
        .then(function(response) {
            if (response.ok)
                return response.json();
            throw new Error('network err')
        })
        .then(function(data) {

            var deviceShown = document.getElementById("additionaDevicesSelect")
            var deviceConfigDiv = document.getElementById("deviceConfigDiv")
            var deviceInstalledDiv = document.getElementById("deviceInstalledDiv")

            deviceShown.innerHTML = ''
            deviceInstalledDiv.innerHTML = ''
            deviceConfigDiv.innerHTML = ''

            for (var eachinstall in data["instances"]) {
                if (data["instances"].hasOwnProperty(eachinstall)) {
                    var installDiv = document.createElement("div")
                    installDiv.setAttribute("class", "horizontalContainer")

                    var nameDiv = document.createElement("div")
                    nameDiv.innerText = data["instances"][eachinstall]["name"]

                    var delButton = document.createElement("button")
                    delButton.innerText = "Remove"
                    var quick = data["instances"][eachinstall]
                    delButton.setAttribute("onclick", "removeDevice(" + quick["id"] + ",'" + quick["config"] + "'," + quick["instance"] + ",'" + quick["name"] + "')")

                    installDiv.appendChild(nameDiv)
                    installDiv.appendChild(delButton)

                    deviceInstalledDiv.appendChild(installDiv)
                }
            }


            for (var eachDevice in data["options"]) {
                if (data["options"].hasOwnProperty(eachDevice)) {
                    var opt = document.createElement("option")
                    opt.value = data["options"][eachDevice]["id"]
                    opt.innerText = data["options"][eachDevice]["name"]

                    deviceShown.appendChild(opt)

                    if (data["options"][eachDevice].hasOwnProperty("config")) {
                        var deviceOptions = data["options"][eachDevice]["config"]

                        if (deviceOptions.length) {
                            addConfigItems(deviceConfigDiv, data["options"][eachDevice]["name"], deviceOptions)

                        }
                    }

                }
            }
            showCorrectDeviceConfig()
        })

}