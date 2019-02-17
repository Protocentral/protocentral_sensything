const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<style>
img {
  display: block;
  margin-left: auto;
  margin-right: auto;
}

.card{
  background-color: #d0d0FF;
  -moz-border-radius: 5px;
  -webkit-border-radius: 5px;
  border-radius: 5px;
  border: 2px solid #000000;
  width: 800px;
  margin: 0 auto;
  padding: 20px
}

</style>

<body>

<img src="https://sensything.protocentral.com/images/sensything_logo.png" style="width:20%;">

 
<div class="card">
  <h4>Protocentral Sensything</h4><br>

  <h4>Sensything Qwiic</h4><br>
  
  <h1>Qwiic Port 1:<span id="QwiicValue">0</span></h1><br>
  <br><a href="http://sensything.protocentral.com/">Sensything.com</a>
</div>
<script>

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
 }, 100); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
   xhttp.onreadystatechange = function() {
   if (this.readyState == 4 && this.status == 200) {
      document.getElementById("QwiicValue").innerHTML =
      this.responseText;
    }
  };
  
  xhttp.open("GET", "readQwiic", true);
  xhttp.send();
}

</script>
</body>
</html>
)=====";
