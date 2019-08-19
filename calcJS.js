function calcSettings() {
    this.m_mode = $("#radioButtons input:checked").val(); // gets value of checked item in radiobuttons group
    this.m_convertTo = document.getElementById("convertTo").value;
    this.m_convertFrom = document.getElementById("convertFrom").value;
}

function calcDisplay() {
    this.m_Display = document.getElementById("TextArea1").value;
}

function calcInput() {
    if (typeof calcInput.m_inputHist === "undefined") {
        var inputs = getCookieValue("inputs");

        if (inputs !== null) {
            try {
                inputs = JSON.parse(inputs);
                calcInput.m_inputHist = inputs.m_Input;
                if (calcInput.m_inputHist === undefined) { // case where inputs is an empty object
                    calcInput.m_inputHist = [];
                }
            } catch (err) {
                console.log("in calcInput() failed to parse input history: " + err);
                calcInput.m_inputHist = [];
            }
        } else {
            calcInput.m_inputHist = [];
        }
    }
    if (typeof calcInput.m_currentIndex === "undefined") { // keeps track of current input index
        if (calcInput.m_inputHist.length === 0) {
            calcInput.m_currentIndex = 0;
        }
        else {
            calcInput.m_currentIndex = calcInput.m_inputHist.length - 1;
        }
    }
    if (typeof calcInput.m_navKeyPressed === "undefined") { // control variable so that it does not increment/decrement on first index update call
        calcInput.m_navKeyPressed = false;
    }

    this.incIndex = function () {
        if (calcInput.m_currentIndex < calcInput.m_inputHist.length - 1 && calcInput.m_navKeyPressed === true) {
            calcInput.m_currentIndex++;
        } else {
            calcInput.m_navKeyPressed = true;
        }
    }
    this.decIndex = function () {
        if (calcInput.m_currentIndex > 0 && calcInput.m_navKeyPressed === true) {
            calcInput.m_currentIndex--;
        } else {
            calcInput.m_navKeyPressed = true;
        }
    }
    this.addInput = function (input) {
        calcInput.m_inputHist[calcInput.m_inputHist.length] = input;
        calcInput.m_currentIndex = calcInput.m_inputHist.length - 1;
        calcInput.m_navKeyPressed = false;
    }
    this.getValue = function () {
        if (calcInput.m_inputHist.length > 0) {
            return calcInput.m_inputHist[calcInput.m_currentIndex];
        } else {
            return "";
        }
    }
    this.getIputHistArr = function () {
        return calcInput.m_inputHist;
    }
    this.setInputHistArr = function (inputHist) {
        calcInput.m_inputHist = inputHist;
        calcInput.m_currentIndex = inputHist.length - 1;
        calcInput.m_navKeyPressed = false;
    }
}

function escapeRegExp(string) { // escapes all special regex characters
    return string.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&");
}

function setCookie() {
    var settings, currentDisplay, cachedDisplay, cachedInputs, inputHist, inputs;
    var pattern = null;
    var inputHistSum, inputHistArr;

    if (document.getElementById("doneGettingCookies").value === "true") { // saves cookies only if function that sets them has finished
        currentDisplay = new calcDisplay();
        cachedDisplay = getCookieValue("display");
        cachedInputs = getCookieValue("inputs");
        settings = new calcSettings();
        inputHist = new calcInput();
        inputs = { m_Input: inputHist.getIputHistArr() };

        if (cachedDisplay !== null) {
            try {
                cachedDisplay = JSON.parse(cachedDisplay);
                pattern = new RegExp("^(" + escapeRegExp(cachedDisplay.m_Display) + ")"); // regex match for presence of cached display
            } catch (err) {
                console.log("In setCookie() failed to parse cached display cookie: " + err);
            }
        }

        if (pattern !== null && pattern.test(currentDisplay.m_Display) === false) {
            currentDisplay.m_Display = cachedDisplay.m_Display + currentDisplay.m_Display; // prepends cached display to current if it exists and not part of current

            if (cachedInputs !== null) {
                try {
                    cachedInputs = JSON.parse(cachedInputs);
                    inputHistArr = inputHist.getIputHistArr();
                    inputHistSum = cachedInputs.m_Input;
                    for (var i = inputHistSum.length, j = 0; j < inputHistArr.length; i++ , j++) {
                        inputHistSum[i] = inputHistArr[j];
                    }
                    inputs.m_Input = inputHistSum;
                } catch (err) {
                    console.log("In setCookie() failed to parse inputs cookie: " + err);
                }
            }
        }

        document.cookie = makeCookie("display", currentDisplay);
        document.cookie = makeCookie("settings", settings);
        document.cookie = makeCookie("inputs", inputs);
    }
}

function getCookie() {
    var cookie = decodeURIComponent(document.cookie);
    var cookieJar = cookie.split(";");
    var cookieObj;
    var cookieObjName;
    var display = document.getElementById("TextArea1");
    var radioButtonGroup = $("#radioButtons input");
    var prefix = radioButtonGroup[0], infix = radioButtonGroup[1], postfix = radioButtonGroup[2], converter = radioButtonGroup[3];
    for (i in cookieJar) {
        try {
            cookieObjName = cookieJar[i].substring(0, cookieJar[i].indexOf("=")).replace(/\s+/g, ""); // gets name of object and clears all white spaces
            cookieObj = JSON.parse(cookieJar[i].substring(cookieJar[i].indexOf("=") + 1)); // get cookie object

            if (cookieObjName === "display") {
                display.value = cookieObj.m_Display;
            }
            else if (cookieObjName === "settings") {
                (cookieObj.m_mode === prefix.value) ? prefix.checked = true : prefix.checked = false;
                (cookieObj.m_mode === infix.value) ? infix.checked = true : infix.checked = false;
                (cookieObj.m_mode === postfix.value) ? postfix.checked = true : postfix.checked = false;
                (cookieObj.m_mode === converter.value) ? converter.checked = true : converter.checked = false;

                document.getElementById("convertToVal").value = cookieObj.m_convertTo.substring(0, 3);
                document.getElementById("convertTo").value = cookieObj.m_convertTo;
                document.getElementById("convertFromVal").value = cookieObj.m_convertFrom.substring(0, 3);
                document.getElementById("convertFrom").value = cookieObj.m_convertFrom;
            }
        } catch (err) {
            console.log("In getCookie() failed to parse cookie: " + err);
        }
    }
    display.scrollTop = display.scrollHeight; // moves scroll bar to the bottom
    document.getElementById("doneGettingCookies").value = "true"; // allows cookies to be set
}

function makeCookie(attribute, value) {
    var date = new Date();
    date.setTime(date.getTime() + (30 * 24 * 60 * 60 * 1000)); // sets expiration date to 30 days from creation
    var expiration = ";expires=" + date.toUTCString() + ";path=/;"; // default expiration date

    return [attribute, "=", JSON.stringify(value), expiration].join("");
}

function getCookieValue(attribute) { // retrieves attribute value from a cookie
    var cookie = decodeURIComponent(document.cookie);
    var cookie = cookie.split(";");
    var attrName;

    for (i in cookie) {
        attrName = cookie[i].substring(0, cookie[i].indexOf("=")).replace(/\s+/g, ""); // gets attribute name from the cookie
        if (attrName === attribute) {
            return cookie[i].substring(cookie[i].indexOf("=") + 1); // returns cookie object
        }
    }

    return null;
}

function disableBtn() {
    var calcButton = document.getElementById("calculateButton");

    if (typeof (Page_ClientValidate) == 'function' && Page_ClientValidate() == true) {
        var inputHist = new calcInput();

        calcButton.style.backgroundImage = "url(/Icons/loading.gif)";
        calcButton.style.backgroundRepeat = "no-repeat";
        calcButton.style.backgroundPosition = "center center";
        //calcButton.removeAttribute("value"); // doing this makes input field to move downwards ~2px on postback???
        calcButton.value = " ";
        calcButton.disabled = true; // disables button on client side

        inputHist.addInput(document.getElementById("Text1").value);
    }
}

function slideDown() {
    var display = document.getElementById("TextArea1");

    display.scrollTop = display.scrollHeight; // moves scroll bar to the bottom
}

function histKeyEvents() { // attached by scriptmanager on backend
    $("#Text1").on("keyup", function (key) { // event for up and down arrow keys
        if (key.which == 38 || key.which == 40) {
            var inputHist = new calcInput();
            var input = document.getElementById("Text1");

            if (key.which == 40) {
                inputHist.incIndex();
            } else {
                inputHist.decIndex();
            }
            input.value = inputHist.getValue();
        }
    });
}

$(document).ready(function () {
    $("#convertTo").click(function () { // toggles convertTo drop down
        $("#convertToDD").toggle();
    });

    $("#convertFrom").click(function () { // toggles convertFrom drop down
        $("#convertFromDD").toggle();
    });

    window.onclick = function (e) { // hides convert drop downs if clicked elsewhere in the window
        if (!e.target.matches("#convertTo")) {
            $("#convertToDD").hide();
        }
        if (!e.target.matches("#convertFrom")) {
            $("#convertFromDD").hide();
        }
    }

    $("#convertToDD label").click(function () { // replaces converTo button name with new value
        document.getElementById("convertTo").value = this.innerHTML + String.fromCharCode("0x25BC");
        document.getElementById("convertToVal").value = this.innerHTML;
    });

    $("#convertFromDD label").click(function () { // replaces convertFrom button name with new value
        document.getElementById("convertFrom").value = this.innerHTML + String.fromCharCode("0x25BC");
        document.getElementById("convertFromVal").value = this.innerHTML;
    });

    $("#controlsToggle img").click(function () { // toggles calculator controls
        var src = document.getElementById("controlsIcon");
        if (src.getAttribute("src") === "Icons\\uparrow.png") {
            src.setAttribute("src", "Icons\\downarrow.png");
            $("#mainContainer").animate({
                height: "473px"
            });
        }
        else {
            src.setAttribute("src", "Icons\\uparrow.png");
            $("#mainContainer").animate({
                height: "618px"
            });
        }
        $("#calcControls").animate({
            height: "toggle"
        });
    });

    $("#Button1").click(function () { // clears main screen, saving any content
        var cachedDisplay = getCookieValue("display");
        var cachedInputs = getCookieValue("inputs");
        var currentDisplay = new calcDisplay();
        var inputHist = new calcInput();
        var inputHistSum, inputHistArr;
        var pattern = null;

        if (cachedDisplay !== null) {
            try {
                cachedDisplay = JSON.parse(cachedDisplay);
                pattern = new RegExp("^(" + escapeRegExp(cachedDisplay.m_Display) + ")");
            } catch (err) {
                console.log("In clear screen event failed to parse cached display cookie: " + err);
            }
        }

        if (pattern !== null && pattern.test(currentDisplay.m_Display) === false) { // case where cached display is not part of current display
            cachedDisplay.m_Display += currentDisplay.m_Display;
            document.cookie = makeCookie("display", cachedDisplay);

            if (cachedInputs !== null) {
                try {
                    cachedInputs = JSON.parse(cachedInputs);
                    inputHistArr = inputHist.getIputHistArr();
                    inputHistSum = cachedInputs.m_Input;
                    for (var i = inputHistSum.length, j = 0; j < inputHistArr.length; i++ , j++) {
                        inputHistSum[i] = inputHistArr[j];
                    }
                    cachedInputs.m_Input = inputHistSum;
                    document.cookie = makeCookie("inputs", cachedInputs);
                } catch (err) {
                    console.log("In clear screen screen event failed to parse inputs cookie: " + err);
                }
            }
        }
        else { // case where cached display is part of current display
            document.cookie = makeCookie("display", currentDisplay)
            document.cookie = makeCookie("inputs", { m_Input: inputHist.getIputHistArr() });
        }

        document.getElementById("TextArea1").value = "";
        document.getElementById("Text1").value = "";
        inputHist.setInputHistArr([]);
    });

    $("#Button2").click(function () { // clears main screen of current session
        var cachedDisplay = getCookieValue("display");
        var cachedInputs = getCookieValue("inputs");
        var inputHist = new calcInput();

        if (cachedDisplay !== null) {
            try {
                cachedDisplay = JSON.parse(cachedDisplay);
            } catch (err) {
                console.log("In clear screen session event failed to parse cached display cookie: " + err);
            }
        }

        if (cachedDisplay !== null) {
            document.getElementById("TextArea1").value = cachedDisplay.m_Display
            if (cachedInputs !== null) {
                try {
                    cachedInputs = JSON.parse(cachedInputs);
                    inputHist.setInputHistArr(cachedInputs.m_Input);
                } catch (err) {
                    console.log("In clear screen session event failed to parse inputs cookie: " + err);
                }
            }
        } else {
            document.getElementById("TextArea1").value = "";
        }
    });

    $("#Button3").click(function () { // clears main screen of history
        var inputs = getCookieValue("inputs");
        var cachedDisplay = getCookieValue("display");
        var currentDisplay = new calcDisplay();
        var inputHist = new calcInput();
        var removeDisplay = "display=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
        var removeInputs = "inputs=; expires=Thu, 01 Jan 1970 00:00:00 UTC; path=/;";
        var inputsDiff = [], inputHistArr;
        var pattern = null;

        if (cachedDisplay !== null) {
            try {
                cachedDisplay = JSON.parse(cachedDisplay);
                pattern = new RegExp("^(" + escapeRegExp(cachedDisplay.m_Display) + ")");
            } catch (err) {
                console.log("In clear screen history event failed to parse cached display cookie: " + err);
            }
        }

        if (cachedDisplay !== null && pattern !== null && pattern.test(currentDisplay.m_Display) === true) {
            document.getElementById("TextArea1").value = currentDisplay.m_Display.substring(cachedDisplay.m_Display.length); // only display current session

            if (inputs !== null) {
                try {
                    inputs = JSON.parse(inputs);
                    inputHistArr = inputHist.getIputHistArr();
                    for (var i = inputs.m_Input.length, j = 0; i < inputHistArr.length; i++ , j++) {
                        inputsDiff[j] = inputHistArr[i];
                    }
                    inputHist.setInputHistArr(inputsDiff);
                } catch (err) {
                    console.log("In clear screen history event failed to parse inputs cookie: " + err);
                }
            }
        }
        document.cookie = removeDisplay; // clears display cookie
        document.cookie = removeInputs;
    });
});