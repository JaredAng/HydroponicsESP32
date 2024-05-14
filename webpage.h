String temperatureC = "--",
       EC = "--", ecLimitLW = "1200.00", ecLimitUP = "1400.00",
       pH = "--", phLimitLW = "5.60", phLimitUP = "6.30",
       timeInterval = "1500", message = "",
solenoid[3] = {"False", "False", "False"};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Automated Hydroponics UI</title>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
     margin-left: auto;
     margin-right: auto;
    }
    h2 { font-size: 3.0rem; }
    div { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    input {
      width: 6rem; 
      font-size: 1.5rem;
      border: none;
      text-align: center;
    }
    form {vertical-align: left;}
    button {width: auto; height: 2rem; font-size: 1.2rem;}
    table, th {
      width: auto;
      height: auto;
      margin-left: auto;
      margin-right: auto;
    }
    td {width: 5rem;}
  </style>
</head>
<body>
  <h2>Automated Hydroponics</h2>
  <br>
  <table>
    <tr>
      <th></th>
      <th>Written to SD Card</th>
      <th></th>
    </tr>
    <tr>
      <td colspan = '3'><p id = "message">%message%</p></td>
    </tr>
  </table>
  <br>
  <table>
    <tr>
      <td colspan = '3'>TEMPERATURE (<span class="units">&deg;C</span>)</td>
    </tr>
    <tr>
      <th></th>
      <th>Reading</th>
      <th></th>
    </tr>
    <tr>
      <td></td>
      <td><span id="temperaturec">%TEMPERATUREC%</span> </td>
      <td></td>
    </tr> 
  </table>
  <br>
  <table>
    <tr>
      <td colspan = '3'>Total Dissolve Solids (<span class="units">ppm</span>)</td>
    </tr>
    <tr>      
      <th>LW Limit</th>
      <th>Reading</th>
      <th>UP Limit</th>
    </tr>
    <tr>
      <td><span id="ecLimitLW">%ecLW%</span></td>
      <td><span id="EC">%EC%</span> </td>
      <td><span id="ecLimitUP">%ecUP%</span></td>
    </tr>
  </table>
  <br>
  <table>
    <tr>
      <td colspan = '3'>Acidity Level (<span class="units">pH</span>)</td>
    </tr>
    <tr>
      <th>LW Limit</th>
      <th>Reading</th>
      <th>UP Limit</th>
    </tr>
    <tr>
      <td><span id="phLimitLW">%pHLW%</span> </td>
      <td><span id="pH">%pH%</span> </td>
      <td><span id="phLimitUP">%pHUP%</span></td>
    </tr>
    
  </table>
  <br>
  <table>
    <tr>
      <td colspan = '3'>Solenoids Is on?</td>
    </tr>
    <tr>
      <th>Acid</th>
      <th>Base</th>
      <th>Nutrients</th>
    </tr>
    <tr>
      <td><span id="solenoid1">%solenoid1%</span> </td>
      <td><span id="solenoid2">%solenoid2%</span></td>
      <td><span id="solenoid3">%solenoid3%</span></td>
    </tr>
  </table>
  <br>
  <table>
    <tr>
      <th colspan = '4'><h3>Settings</h3></th>
    </tr>
    <tr>
      <td></td>
      <td>LW Limit</td>
      <td>UP Limit</td>
    </tr>
    <tr class = "formTBL">
      <form action="/" method = 'POST'>
        <td><label>TDS (ppm)</label></td>
        <td><input type = 'number' name = 'ecLimitLW' id = 'ecLimitLWIN' min = 0 step = 0.01 placeholder="1200.0"></td>
        <td><input type = 'number' name = 'ecLimitUP' id = 'ecLimitUPIN' min = 0 step = 0.01 placeholder="1400.0"></td>
        <td><button type='submit' value='Submit'>Set EC</button></td>
      </form>
    </tr>
    <tr class = "formTBL">
      <form action="/" method = 'POST'>
        <td><label>Acidity (pH)</label></td>
        <td><input type = 'number' name = 'phLimitLW' id = 'phLimitlWIN' min = 0 step = 0.01 max = 100 placeholder="5.6"></td>
        <td><input type = 'number' name = 'phLimitUP' id = 'phLimitUPIN' min = 0 step = 0.01 max = 100 placeholder="6.3" ></td>
        <td><button type='submit' value='Submit'>Set pH</button></td>
      </form>
    </tr>
  </table>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, %timeInterval%) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pH").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ph", true);
  xhttp.send();
}, %timeInterval%) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("EC").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ec", true);
  xhttp.send();
}, %timeInterval%) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("phLimitUP").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Limit/ph/up", true);
  xhttp.send();
}, %timeInterval%);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("phLimitLW").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Limit/ph/lw", true);
  xhttp.send();
}, %timeInterval%);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ecLimitUP").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Limit/ec/up", true);
  xhttp.send();
}, %timeInterval%);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ecLimitLW").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Limit/ec/lw", true);
  xhttp.send();
}, %timeInterval%);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("solenoid1").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/solenoids/acid", true);
  xhttp.send();
}, %timeInterval%);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("solenoid2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/solenoids/base", true);
  xhttp.send();
}, %timeInterval%);
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("solenoid3").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/solenoids/nutrient", true);
  xhttp.send();
}, %timeInterval%);

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("message").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/message", true);
  xhttp.send();
}, %timeInterval%);
</script>
</html>)rawliteral";

// Replaces placeholder
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATUREC") {
    return temperatureC;
  } else if (var == "EC") {
    return EC;
  }  else if (var == "pH") {
    return pH;
  } else if (var == "timeInterval"){
    return timeInterval;  
  } else if (var == "solenoid1"){
    return solenoid[0];  
  } else if (var == "solenoid2"){
    return solenoid[1];  
  } else if (var == "solenoid3"){
    return solenoid[2];  
  } else if (var == "ecLW"){
    return ecLimitLW;  
  }else if (var == "ecUP"){
    return ecLimitUP;  
  }else if (var == "pHLW"){
    return phLimitLW;  
  }else if (var == "pHUP"){
    return phLimitUP;  
  }else if (var == "message"){
    return message;  
  }
  return "No Data";
}
