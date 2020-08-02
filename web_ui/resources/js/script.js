window.onload =
    () => {
      console.log("Initialized");
      setInterval(readDefaultGw, 3000);
      setInterval(readInterfaceStatus, 3000);
    }

setDefaultGw =
    (event) => {
      let ifNameElement =
          event.target.parentNode.getElementsByClassName("if_name")[0];
      let ifName = ifNameElement.textContent;
      let forceGwRequest = new XMLHttpRequest();
      forceGwRequest.open('GET', '/set_default_gw?interface=' + ifName);
      forceGwRequest.onreadystatechange = function() {
        if (forceGwRequest.readyState === 4 && forceGwRequest.status == 200) {
          let parsedResponse = JSON.parse(forceGwRequest.responseText);
          document.getElementById('default_gw').innerHTML =
              "Default Gateway: " + parsedResponse.default_gw;
        }
      };
      forceGwRequest.send();
    }

readDefaultGw =
    () => {
      let gwRequest = new XMLHttpRequest();
      gwRequest.open('GET', '/get_default_gw');
      gwRequest.onreadystatechange = function() {
        if (gwRequest.readyState === 4 && gwRequest.status == 200) {
          let parsedResponse = JSON.parse(gwRequest.responseText);
          document.getElementById('default_gw').innerHTML =
              "Default Gateway: " + parsedResponse.default_gw;
        }
      };
      gwRequest.send();
    }

readInterfaceStatus = () => {
  let ifRequest = new XMLHttpRequest();
  ifRequest.open('GET', '/get_interface_status');
  ifRequest.onreadystatechange = function() {
    if (ifRequest.readyState === 4 && ifRequest.status == 200) {
      let parsedResponse = JSON.parse(ifRequest.responseText);
      let ifStatusDiv = document.getElementById('if_status');
      ifStatusDiv.innerHTML = "";
      parsedResponse.interfaceStatus.forEach((element) => {
        let newDiv = document.createElement("div");
        let ifNameText = document.createElement("span");
        ifNameText.setAttribute("class", "if_name");
        ifNameText.innerHTML = element.ifName;
        let ifStatusText = document.createTextNode(element.status);
        let setDefaultGwButton = document.createElement("button");
        setDefaultGwButton.innerText = "Set Default";
        setDefaultGwButton.onclick = setDefaultGw;
        newDiv.appendChild(ifNameText);
        newDiv.appendChild(document.createTextNode(" - "));
        newDiv.appendChild(ifStatusText);
        newDiv.appendChild(setDefaultGwButton);
        ifStatusDiv.appendChild(newDiv);
      });
    }
  };
  ifRequest.send();
}
