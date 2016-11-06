$('#on_off_checkbox').checkbox();
$('#hide_other_users_logon_tiles_checkbox').checkbox();
$('#prohibit_fallback_credential_provider_checkbox').checkbox();

for (let i in settings) {
  if (settings[i] == '0') {
    $('#' + i + '_checkbox').checkbox('uncheck');
  }
  else if (settings[i] == '1') {
    $('#' + i + '_checkbox').checkbox('check');
  }
}

$('#on_off_checkbox').checkbox({
  onChecked: () => {
    $('#on_off_checkbox').checkbox('set disabled');
    $('#hide_other_users_logon_tiles_checkbox').checkbox('set enabled');
    $('#prohibit_fallback_credential_provider_checkbox').checkbox('set enabled');

    addon.set_setting_registry_value(settings_enumeration['on_off'], 1);

    addon.set_credential_provider_registry_keys_and_values();

    $('#on_off_checkbox').checkbox('set enabled');
  },
  onUnchecked: () => {
    $('#on_off_checkbox').checkbox('set disabled');
    $('#hide_other_users_logon_tiles_checkbox').checkbox('uncheck');
    $('#hide_other_users_logon_tiles_checkbox').checkbox('set disabled');
    $('#prohibit_fallback_credential_provider_checkbox').checkbox('uncheck');
    $('#prohibit_fallback_credential_provider_checkbox').checkbox('set disabled');

    addon.set_setting_registry_value(settings_enumeration['on_off'], 0);

    addon.delete_credential_provider_registry_keys();

    $('#on_off_checkbox').checkbox('set enabled');
  }
});

$('#hide_other_users_logon_tiles_checkbox').checkbox({
  onChecked: () => {
    addon.set_setting_registry_value(settings_enumeration['hide_other_users_logon_tiles'], 1);

    addon.set_excluded_credential_provider_registry_value(1);
  },
  onUnchecked: () => {
    addon.set_setting_registry_value(settings_enumeration['hide_other_users_logon_tiles'], 0);

    addon.set_excluded_credential_provider_registry_value(0);
  }
});

$('#prohibit_fallback_credential_provider_checkbox').checkbox({
  onChecked: () => {
    addon.set_setting_registry_value(settings_enumeration['prohibit_fallback_credential_provider'], 1);

    addon.set_fallback_credential_provider_registry_value(1);
  },
  onUnchecked: () => {
    addon.set_setting_registry_value(settings_enumeration['prohibit_fallback_credential_provider'], 0);

    addon.set_fallback_credential_provider_registry_value(0);
  }
});
