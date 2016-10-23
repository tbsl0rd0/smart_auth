var initial_items = [
  {
    name: 'GoogleOTP',
    type: 'REG_SZ',
    value: '0'
  },
  {
    name: 'SmartIDCard',
    type: 'REG_SZ',
    value: '0'
  },
  {
    name: 'Donglein',
    type: 'REG_SZ',
    value: '0'
  }
];

registry_key.values(function(error, items) {
  var exists = false;
  for (var key in initial_items) {
    for (var key_2 in items) {
      if (initial_items[key].name == items[key_2].name) {
        exists = true;
      }
    }
    if (exists == true) {
      exists = false;
    }
    else if (exists == false) {
      registry_key.set(initial_items[key].name, initial_items[key].type, initial_items[key].value, function() {});
    }
  }
});

var initial_items_2 = [
  {
    name: 'on_off',
    type: 'REG_SZ',
    value: '0'
  },
  {
    name: 'hide_other_users_logon_tile',
    type: 'REG_SZ',
    value: '0'
  },
  {
    name: 'prohibit_fallback_credential_provider',
    type: 'REG_SZ',
    value: '0'
  }
];

registry_key_2.values(function(error, items) {
  var exists = false;
  for (var key in initial_items_2) {
    for (var key_2 in items) {
      if (initial_items_2[key].name == items[key_2].name) {
        exists = true;
      }
    }
    if (exists == true) {
      exists = false;
    }
    else if (exists == false) {
      registry_key_2.set(initial_items_2[key].name, initial_items_2[key].type, initial_items_2[key].value, function() {});
    }
  }
});
