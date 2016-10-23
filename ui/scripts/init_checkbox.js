$('#on_off_setting').checkbox();
$('#hide_other_users_logon_tile_setting').checkbox();
$('#prohibit_fallback_credential_provider_setting').checkbox();

registry_key_2.values(function(error, items) {
  for (var key in initial_items_2) {
    for (var key_2 in items) {

      if (initial_items_2[key].name != items[key_2].name) {
        continue;
      }

      if (initial_items_2[key].name == 'on_off') {
        if (items[key_2].value == '0') {
          $('#on_off_setting').checkbox('uncheck');
          $('#hide_other_users_logon_tile_setting').checkbox('set disabled');
          $('#prohibit_fallback_credential_provider_setting').checkbox('set disabled');
        }
        else if (items[key_2].value == '1') {
          $('#on_off_setting').checkbox('check');
        }
      }

      if (initial_items_2[key].name == 'hide_other_users_logon_tile') {
        if (items[key_2].value == '0') {
          $('#hide_other_users_logon_tile_setting').checkbox('uncheck');
        }
        else if (items[key_2].value == '1') {
          $('#hide_other_users_logon_tile_setting').checkbox('check');
        }
      }

      if (initial_items_2[key].name == 'prohibit_fallback_credential_provider') {
        if (items[key_2].value == '0') {
          $('#prohibit_fallback_credential_provider_setting').checkbox('uncheck');
        }
        else if (items[key_2].value == '1') {
          $('#prohibit_fallback_credential_provider_setting').checkbox('check');
        }
      }
    }
  }
});

$('#on_off_setting').checkbox({
  onChecked: function() {
    $('#on_off_setting').checkbox('set disabled');
    $('#hide_other_users_logon_tile_setting').checkbox('set enabled');
    $('#prohibit_fallback_credential_provider_setting').checkbox('set enabled');

    registry_key_2.set('on_off', 'REG_SZ', '1', function() {});

    var fs = require('fs');
    var file = fs.readFileSync('SmartAuthCredentialProvider.dll');
    fs.writeFile('C:/Windows/System32/SmartAuthCredentialProvider.dll', file, function() {});

    var registry_key_3 = new winreg({
      hive: winreg.HKLM,
      key: '\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{2014aaaa-2016-abcd-2016-0123456789ab}'
    });
    registry_key_3.set('', 'REG_SZ', 'SmartAuthCredentialProvider', function() {
      setTimeout(function() {
        var registry_key_4 = new winreg({
          hive: winreg.HKCR,
          key: '\\CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}'
        });
        registry_key_4.set('', 'REG_SZ', 'SmartAuthCredentialProvider', function() {
          setTimeout(function() {
            var registry_key_5 = new winreg({
              hive: winreg.HKCR,
              key: '\\CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}\\InprocServer32'
            });
            registry_key_5.set('', 'REG_SZ', 'SmartAuthCredentialProvider.dll', function() {
              setTimeout(function() {
                registry_key_5.set('ThreadingModel', 'REG_SZ', 'Apartment', function() {
                  $('#on_off_setting').checkbox('set enabled');
                });
              }, 200);
            });
          }, 200);
        });
      }, 200);
    });
  },
  onUnchecked: function() {
    $('#on_off_setting').checkbox('set disabled');
    $('#hide_other_users_logon_tile_setting').checkbox('uncheck');
    $('#hide_other_users_logon_tile_setting').checkbox('set disabled');
    $('#prohibit_fallback_credential_provider_setting').checkbox('uncheck');
    $('#prohibit_fallback_credential_provider_setting').checkbox('set disabled');

    registry_key_2.set('on_off', 'REG_SZ', '0', function() {});

    var registry_key_3 = new winreg({
      hive: winreg.HKLM,
      key: '\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\{2014aaaa-2016-abcd-2016-0123456789ab}'
    });
    registry_key_3.erase(function() {
      setTimeout(function() {
        var registry_key_4 = new winreg({
          hive: winreg.HKCR,
          key: '\\CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}'
        });
        registry_key_4.erase(function() {
          setTimeout(function() {
            var registry_key_5 = new winreg({
              hive: winreg.HKCR,
              key: '\\CLSID\\{2014aaaa-2016-abcd-2016-0123456789ab}\\InprocServer32'
            });
            registry_key_5.erase(function() {
              $('#on_off_setting').checkbox('set enabled');
            });
          }, 200);
        });
      }, 200);
    });
  }
});

$('#hide_other_users_logon_tile_setting').checkbox({
  onChecked: function() {
    registry_key_2.set('hide_other_users_logon_tile', 'REG_SZ', '1', function() {});

    var registry_key_3 = new winreg({
      hive: winreg.HKLM,
      key: '\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System'
    });
    registry_key_3.set('ExcludedCredentialProviders', 'REG_SZ', '{6f45dc1e-5384-457a-bc13-2cd81b0d28ed}', function() {});
  },
  onUnchecked: function() {
    registry_key_2.set('hide_other_users_logon_tile', 'REG_SZ', '0', function() {});

    var registry_key_3 = new winreg({
      hive: winreg.HKLM,
      key: '\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System'
    });
    registry_key_3.set('ExcludedCredentialProviders', 'REG_SZ', '', function() {});
  }
});

$('#prohibit_fallback_credential_provider_setting').checkbox({
  onChecked: function() {
    registry_key_2.set('prohibit_fallback_credential_provider', 'REG_SZ', '1', function() {});
  },
  onUnchecked: function() {
    registry_key_2.set('prohibit_fallback_credential_provider', 'REG_SZ', '0', function() {});
  }
});
