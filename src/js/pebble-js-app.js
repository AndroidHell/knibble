Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://cdn.rawgit.com/kaveet/knibble/6c626c2d83dd6cc2488bd49f23bc629a7a383b02/config-page/config.html';
  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  var backgroundColor = configData['background_color'];

  var dict = {};
  if(configData['high_contrast'] === true) {
    dict[0] = 1;  // Send a boolean as an integer
  }
  else {
    dict[0] = 0;
  }

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});
