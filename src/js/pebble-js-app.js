Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'https://cdn.rawgit.com/kaveet/knibble/5d2257c91f32d40179b43f12450747a860071e3e/config-page/config.html';
  //console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  //console.log('Configuration page returned: ' + JSON.stringify(configData));

  var highContrastMode = configData['high_contrast'];

  var dict = {};
  if(highContrastMode === 1) {
    dict[0] = 1;  // Send a boolean as an integer
  }
  else {
    dict[0] = 0;
  }

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    //console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    //console.log('Send failed!');
  });
});
