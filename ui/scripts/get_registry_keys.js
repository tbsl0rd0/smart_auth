var winreg = require('winreg');

var registry_key = new winreg({
  hive: winreg.HKLM,
  key: '\\SOFTWARE\\SmartAuth'
});

var registry_key_2 = new winreg({
  hive: winreg.HKLM,
  key: '\\SOFTWARE\\SmartAuth\\settings'
});
