// when testing these web pages from the PC ... base should point to the esp mule
// chrome --disable-web-security --user-data-dir="[some directory here]"
//let base = "http://192.168.4.1"
let base = ""

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

// generic

function postSettingReturnPromise(config, url) {

    const options = {
        method: 'post',
        headers: {
            'Content-type': 'text/json'
        },
        body: JSON.stringify(config)
    }

    return fetch(url, options)

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
    if (!selector.length)
        return

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

function addDeviceAndConfig() {
    // get id and stringify the options
    var jsonData = {}
    var devType = document.getElementById("additionaDevicesSelect");
    jsonData["id"] = devType.value;

    var options = getDeviceConfigsFieldset()
    var selector = devType
    var selected = selector.options[selector.selectedIndex].innerText



    for (var loop = 0; loop < options.length; loop++) {
        var item = options[loop]

        if (!item.hidden) {
            var jsonConfig = {}

            // <legend>
            // each line is a <div><div><input|select></></>

            var lineItems = item.children

            // skip the legend
            for (var eachChild = 1; eachChild < lineItems.length; eachChild++) {

                var optname = lineItems[eachChild].children[0].innerText;
                var optValue = null
                var valueHolder = lineItems[eachChild].children[1]

                switch (valueHolder.tagName) {
                    case "SELECT":
                        optValue = parseInt(valueHolder.options[valueHolder.selectedIndex].value)
                        break;
                }

                jsonConfig[optname] = optValue
            }



            jsonData["config"] = jsonConfig

            break
        }
    }

    let url = base + '/json/devices/add'
    postSettingReturnPromise(jsonData, url).then(function(response) {
        // then repop
        populateConfiguredDevices()
    })



}

function removeDevice(id, config, instance, name) {

    if (confirm("About to remove device '" + name + "'")) {
        var jsonData = {}
        jsonData["id"] = id;
        jsonData["config"] = config;
        jsonData["instance"] = instance;
        let url = base + '/json/devices/del'
        postSettingReturnPromise(jsonData, url).then(function(response) {
            populateConfiguredDevices()

        })


    }


}