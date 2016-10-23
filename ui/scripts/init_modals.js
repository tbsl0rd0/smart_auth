$('#google_otp_settings_ok').on('click', function() {
  $('#google_otp_settings_modal').modal('hide');
  registry_key.set("GoogleOTP", 'REG_SZ', '1', function() {
    registry_key.set("GoogleOTPKey", 'REG_SZ', google_otp_key, function() {});
  });
});

$('#google_otp_settings_cancel').on('click', function() {
  $('#google_otp_settings_modal').modal('hide');
  $('.google_otp_item').remove();
  var html = '' +
  '<div class="item step google_otp_item" style="cursor: move;" value="GoogleOTP">' +
    '<i class="icon wait"></i>' +
    '<div class="content">' +
      '<div class="title">' +
        'Google OTP' +
      '</div>' +
      '<div class="description">' +
        'Need to download google OTP app' +
      '</div>' +
    '</div>' +
  '</div>';
  $('#available_auth_stack_items').prepend(html);
});
