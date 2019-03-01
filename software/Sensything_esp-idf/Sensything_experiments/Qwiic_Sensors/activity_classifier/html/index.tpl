<html>
<head><title>Sensything - Home</title>
<link rel="stylesheet" type="text/css" href="./style.css">

<script>
function startTime() {
  var today = new Date();
  var h = today.getHours();
  var m = today.getMinutes();
  var s = today.getSeconds();
  m = checkTime(m);
  s = checkTime(s);
  document.getElementById('txt').innerHTML =
  h + ":" + m + ":" + s;
  var t = setTimeout(startTime, 500);
}
function checkTime(i) {
  if (i < 10) {i = "0" + i};  // add zero in front of numbers < 10
  return i;
}
</script>

</head>


<body onload="startTime()">


<div id="txt"  style="position: absolute; top:5px; left:1200px; width:300px; height:60px"></div>


</body>

<body>

<div class="flex-container-panel">
   <img style="width:30%; " src="./sensything_pc.svg">
   <svg width="0" height="30">
        </svg>
  </div>

<h1>Activity Classifier using Sensything</h1>
<div id="main">

<p>

<h2>Index</h2>
<ul>
<li><a href="websocket1">Activity Classifier Data</a></li>
</ul>
</p>
<br/>
<br/>

</div>
</body></html>
