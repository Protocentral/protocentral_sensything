const char MAIN_page[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<style>
.card{
    max-width: 1000px;
     min-height: 500px;
     background: #00bfff;
     padding: 30px;
     box-sizing: border-box;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<body>
 
<div class="card">
  <h4>Protocentral Sensything</h4><br>
  <h1>Sensor Value:<span id="ADCValue">0</span></h1><br>
  <br><a href="http://sensything.protocentral.com/">Sensything.com</a>
</div>
<script>
 
setInterval(function() {
 
  getData();
}, 2000); // update rate
 
function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
  if (this.readyState == 4 && this.status == 200) {
  document.getElementById("ADCValue").innerHTML =
  this.responseText;
  }
   };

  xhttp.open("GET", "readADC", true);
   xhttp.send();
}
</script>
</body>
</html>
)";
