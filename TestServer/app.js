var express = require('express');
var app = express();
app.use(express.static(__dirname));

app.get('/:username/:password', function(req, res){
	console.log("request received");
	var username = req.params.username;
	var password = req.params.password;
	res.send('username: ' + username + '\npassword: ' + password);
});

app.listen(8080);

console.log("server started!");