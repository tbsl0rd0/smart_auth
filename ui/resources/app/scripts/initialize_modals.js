$('#google_otp_key_setting_confirm').on('click', () => {
  authentication_factors['GoogleOTP'] = '1';

  reset_items();

  $('#google_otp_key_setting_modal').modal('hide');

  addon.set_authentication_factor_registry_value(authentication_factors_enumeration['GoogleOTP'], 1);

  addon.set_google_otp_key_registry_value($('#google_otp_key_setting_segment').text());
});

$('#google_otp_key_setting_cancel').on('click', () => {
  authentication_factors['GoogleOTP'] = '0';

  reset_items();

  $('#google_otp_key_setting_modal').modal('hide');
});
