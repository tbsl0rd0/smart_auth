const reset_items = () => {
  $('#current_authentication_factor_list').text('');
  $('#current_authentication_factor_list_2').text('');
  $('#available_authentication_factor_list').text('');

  for (let i in authentication_factors) {
    if (authentication_factors[i] == '0') {
      $('#available_authentication_factor_list').prepend(item_htmls[i]);
    }
    else if (authentication_factors[i] == '1') {
      $('#current_authentication_factor_list').prepend(item_htmls[i]);
      $('#current_authentication_factor_list_2').prepend(item_htmls[i]);
    }
  }
}

reset_items();

new Sortable(document.getElementById('current_authentication_factor_list_2'), {
  animation: 200,
  group: 'authentication_factor_list',
  onAdd: (event) => {
    const item = event.item;

    if (item.getAttribute('value') == 'GoogleOTP') {
      const execFileSync = require('child_process').execFileSync;

      $('#google_otp_key_setting_segment').text(execFileSync('google_otp/google_otp_key_generator.exe'));

      $('#google_otp_key_setting_modal').modal('setting', 'closable', false).modal('show');
    }
    else {
      authentication_factors[item.getAttribute('value')] = '1';

      reset_items();

      addon.set_authentication_factor_registry_value(authentication_factors_enumeration[item.getAttribute('value')], 1);
    }

    if (addon.get_authentication_factor_registry_value(authentication_factors_enumeration['Donglein']) == '1' || addon.get_authentication_factor_registry_value(authentication_factors_enumeration['SmartIDCard']) == '1') {
      addon.set_authentication_factor_registry_value(authentication_factors_enumeration['HardwareAuth'], 1);
    }
  },
  onRemove: (event) => {
    const item = event.item;

    authentication_factors[item.getAttribute('value')] = '0';

    reset_items();

    addon.set_authentication_factor_registry_value(authentication_factors_enumeration[item.getAttribute('value')], 0);

    if (addon.get_authentication_factor_registry_value(authentication_factors_enumeration['Donglein']) == '0' && addon.get_authentication_factor_registry_value(authentication_factors_enumeration['SmartIDCard']) == '0') {
      addon.set_authentication_factor_registry_value(authentication_factors_enumeration['HardwareAuth'], 0);
    }
  }
});

new Sortable(document.getElementById('available_authentication_factor_list'), {
  animation: 200,
  group: 'authentication_factor_list'
});
