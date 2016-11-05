$('#google_otp_key_setting_confirm').on('click', function() {
  $('#google_otp_key_setting_modal').modal('hide');

  authentication_factors['GoogleOTP'] = '1';

  addon.set_authentication_factor_registry_value(authentication_factors_enumeration['GoogleOTP'], 1);

  addon.set_google_otp_key_registry_value($('#google_otp_key_setting_segment').text());
});

$('#google_otp_key_setting_cancel').on('click', function() {
  $('#google_otp_key_setting_modal').modal('hide');

  authentication_factors['GoogleOTP'] = '0';

  $('.google_otp_item').remove();

  $('#available_authentication_factor_list').prepend(google_otp_item_html);
});
