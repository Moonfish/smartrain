var http = require('http');
var fs = require('fs');
var url = require('url');

function replaceAll(str, mapObj){
    var re = new RegExp(Object.keys(mapObj).join('|'), 'gi');

    return str.replace(re, function(matched){
        return mapObj[matched];
    });
}

function getDataFromStatusFile(){
  var out;

  try{
    var buf = fs.readFileSync('status.txt');
    out = JSON.parse(buf);
  }  
  catch(err){
    out = {statusT:"Ready"}
  }

  return out;
}

function postCommand(str){
   fs.writeFileSync('command.txt', str);
}

function getDataFromSettingsFile(){
  
  var out;

  try{ 
    var buf = fs.readFileSync('settings.txt');
    out = JSON.parse(buf);
  }
  catch(err){
      out = {
      startTimeT:"05:00",
      disableT:"",
      runtime1T:"10",
      runtime2T:"10",
      runtime3T:"10",
      runtime4T:"10"};
    };

  return out;
}

function updateFromForm(res, body){
  // Handle embedded colons
  body = body.replace('%3A', ':');
 
  // Break into parts
  parts = body.split('&');

  var j = '{';

  for (var i = 0; i < parts.length; i++){
    if (i < 5){
      var l = parts[i].split('=');
      if (i != 0){
         j = j + ',';}
      j = j + '\n"';
      j = j + l[0];  
      j = j + 'T": "';
      j = j + l[1];
      j = j + '"';
    }// Do not write form buttons.
  }
  
  j = j + '\n}\n';

  var button = parts[5].split('=')[0];

  if (button == 'update'){
    fs.writeFileSync('settings.txt', j);
    var buf = fs.readFileSync('updated.html');
    var html = buf.toString('utf-8');
    res.end(html);
    return;
  }
 
  if (button == 'run1'){
    fs.writeFileSync('command.txt','R1');
  }
  else if (button == 'run2'){
    fs.writeFileSync('command.txt','R2');
  }
  else if (button == 'run3'){
    fs.writeFileSync('command.txt','R3');
  }
  else if (button == 'run4'){
    fs.writeFileSync('command.txt','R4');
  }
  else{
    fs.writeFileSync('command.txt','S');
  }
 

  // Refresh page
  res.writeHead(302, {'Location': 'index.html'});
  res.end();

  return;
}

function onIndex(req, res){

  if (req.method == 'GET'){
    var buf = fs.readFileSync('index.html');
    var html = buf.toString('utf-8');

    var map = getDataFromSettingsFile();
    html = replaceAll(html, map);

    map = getDataFromStatusFile();
    html = replaceAll(html, map);

    res.setHeader('Content-Type', 'text/html');
    res.end(html);
    return;
  }

  if (req.method == 'POST'){
    var body='';

    req.on('data', function (chunk) {
      body += chunk;
    });

    req.on('end', function() {
      updateFromForm(res, body);
    });
  }

}

function sendDebugMsg(res, text){
  var s = '<html><head>Debug</head><body><h1>' + text;
  s = s + '</h1></body></html>'
  res.setHeader('Content-Type', 'text/html'); 
  res.end(s);
}

function sendFile(name, res){

  fs.readFile(name, function(err, text){
    res.setHeader('Content-Type', 'text/html');
    res.end(text);
  });

}

// * * * Start Program * * * 
http.createServer(function (req, res) {

  var u = url.parse(req.url, true);

  if (u.pathname == '/rain.jpg') {
    sendFile('rain.jpg', res);
    return;
  }

  if(u.pathname == '/' || u.pathname == '/index.html') {
    onIndex(req, res);
    return;
  }

  res.end('<html><head></head><body><p>Invalid Address: ' + req.url.toString() + '</p></body></html>');
  

}).listen(80);

console.log('Server started');

