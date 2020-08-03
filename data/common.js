// when testing these web pages from the PC ... base should point to the esp mule
// chrome --disable-web-security --user-data-dir="[some directory here]"
//let base = "http://192.168.42.121"
let base = "."

function headerLoaded() {
    let url = base + "/json/config";
    var iframe = window.frames.header;
    fetch(url).then((function (response) {
        return response.json()
    })).then((function (data) {
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

function addConfigItems(container, name, configItems, hideit=true)
{
    var configFieldSet=document.createElement("fieldset")
    var configLegend=document.createElement("legend")
    configLegend.innerText=name;
    configFieldSet.appendChild(configLegend)


    configItems.forEach(function(item) {

        var configDiv=document.createElement("div")
        configDiv.className="horizontalContainer"

        configFieldSet.appendChild(configDiv)

        var configName=document.createElement("div")
        configName.innerText=item["name"]

        configDiv.appendChild(configName)

        switch(item["type"])
        {
            case "select":
                var input=document.createElement("select")

                for(var eachOpt in item["options"])
                {
                    if(item["options"].hasOwnProperty(eachOpt))
                    {
                        var opt=document.createElement("option")
                        opt.value=item["options"][eachOpt]["value"]
                        opt.innerText=item["options"][eachOpt]["name"]

                        input.appendChild(opt)

                    }
                }

                configDiv.appendChild(input)
                break;
            case "text":
                var input=document.createElement("input")
                input.type="text"
                input.name=item["name"]
                if(item.hasOwnProperty("default"))
                {
                    input.value=item["default"]
                }
                configDiv.appendChild(input)
                break
            case "number":
                var input=document.createElement("input")
                input.type="number"
                input.name=item["name"]
                if(item.hasOwnProperty("default"))
                    {
                        input.value=item["default"]
                    }
                configDiv.appendChild(input)
                break

        }


    });

    configFieldSet.hidden=hideit

    container.appendChild(configFieldSet)

    
}