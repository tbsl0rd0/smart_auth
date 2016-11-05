let set_items = function() {
  for (let i in authentication_factors) {
    let html;
    if (i == 'Donglein') {
      html = donglein_item_html;
    }
    else if (i == 'GoogleOTP') {
      html = google_otp_item_html;
    }
    else if (i == 'SmartIDCard') {
      html = smart_id_card_item_html;
    }

    if (authentication_factors[i] == '0') {
      $('#available_authentication_factor_list').prepend(html);
    }
    else if (authentication_factors[i] == '1') {
      $('#current_authentication_factor_list').prepend(html);
      $('#current_authentication_factor_list_2').prepend(html);
    }
  }
}

set_items();

let element = document.getElementById('current_authentication_factor_list_2');
new Sortable(element, {
  animation: 200,
  group: 'group',
  onAdd: function(event) {
    let item = event.item;
    if (item.getAttribute('value') == 'GoogleOTP') {
      const execFileSync = require('child_process').execFileSync;
      $('#google_otp_key_setting_segment').text(execFileSync('google_otp_key_generator.exe'));

      $('#google_otp_key_setting_modal').modal('setting', 'closable', false).modal('show');
    }
    else {
      authentication_factors[item.getAttribute('value')] = '1';

      addon.set_authentication_factor_registry_value(authentication_factors_enumeration[item.getAttribute('value')], 1);
    }

    if (addon.get_authentication_factor_registry_value(authentication_factors_enumeration['Donglein']) == '1' || addon.get_authentication_factor_registry_value(authentication_factors_enumeration['SmartIDCard']) == '1') {
      addon.set_authentication_factor_registry_value(authentication_factors_enumeration['HardwareAuth'], 1);
    }
  },
  onRemove: function(event) {
    let item = event.item;
    addon.set_authentication_factor_registry_value(authentication_factors_enumeration[item.getAttribute('value')], 0);

    authentication_factors[item.getAttribute('value')] = '0';

    if (addon.get_authentication_factor_registry_value(authentication_factors_enumeration['Donglein']) == '0' && addon.get_authentication_factor_registry_value(authentication_factors_enumeration['SmartIDCard']) == '0') {
      addon.set_authentication_factor_registry_value(authentication_factors_enumeration['HardwareAuth'], 0);
    }
  }
});

let element_2 = document.getElementById('available_authentication_factor_list');
new Sortable(element_2, {
  animation: 200,
  group: 'group'
});
