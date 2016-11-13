let donglein_key;

$('#google_otp_key_setting_confirm').on('click', () => {
  authentication_factors['GoogleOTP'] = '1';

  reset_items();

  $('#google_otp_key_setting_modal').modal('hide');

  addon.set_authentication_factor_registry_value(authentication_factors_enumeration['GoogleOTP'], 1);

  addon.set_google_otp_key_registry_value($('#google_otp_key_setting_segment').text());
});

$('.google_otp_key_setting_cancel').on('click', () => {
  authentication_factors['GoogleOTP'] = '0';

  reset_items();

  $('#google_otp_key_setting_modal').modal('hide');
});

$('#donglein_key_setting_next').on('click', () => {
  $('#donglein_key_setting_next').addClass('disabled');
  $('#donglein_key_setting_next').addClass('loading');

  $('#donglein_key_setting_step_1').removeClass('active');

  $('#donglein_key_setting_step_2').removeClass('disabled');
  $('#donglein_key_setting_step_2').addClass('active');

  setTimeout(() => {
    donglein_key = '';

    let s = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';

    for(let i = 0; i < 16; i++) {
      donglein_key += s.charAt(Math.floor(Math.random() * s.length));
    }

    let s2 = addon.set_donglein_key(donglein_key);

    if (s2 == 'success') {
      $('#donglein_key_setting_step_2').removeClass('active');

      $('#donglein_key_setting_step_3').removeClass('disabled');
      $('#donglein_key_setting_step_3').addClass('active');

      $('#donglein_key_setting_next').removeClass('loading');
      $('#donglein_key_setting_next').removeClass('disabled');

      $('#donglein_key_setting_next').css('display', 'none');
      $('#donglein_key_setting_confirm').css('display', 'inline-block');
    }
    else if (s2 == 'can\'t connect') {
      $('#donglein_key_setting_step_2').removeClass('active');

      $('#donglein_key_setting_step_3').removeClass('disabled');
      $('#donglein_key_setting_step_3').addClass('active');

      $('#donglein_key_setting_step_3 > i').removeClass('checkmark');
      $('#donglein_key_setting_step_3 > i').addClass('close');

      $('#donglein_key_setting_step_3 > div > div:nth-child(1)').text('Fail');
      $('#donglein_key_setting_step_3 > div > div:nth-child(2)').text('Fail to set donglein key ...');

      $('#donglein_key_setting_next').removeClass('loading');
      $('#donglein_key_setting_next').removeClass('disabled');

      $('#donglein_key_setting_next').css('display', 'none');
    }
  }, 2000);
});

$('#donglein_key_setting_confirm').on('click', () => {
  $('#donglein_key_setting_modal').modal('hide');

  reset_donglein_key_setting_modal();

  authentication_factors['Donglein'] = '1';

  reset_items();

  addon.set_authentication_factor_registry_value(authentication_factors_enumeration['Donglein'], 1);

  addon.set_authentication_factor_registry_value(authentication_factors_enumeration['HardwareAuth'], 1);

  addon.set_donglein_key_registry_value(donglein_key);
});

$('.donglein_key_setting_cancel').on('click', () => {
  $('#donglein_key_setting_modal').modal('hide');

  reset_donglein_key_setting_modal();

  authentication_factors['Donglein'] = '0';

  reset_items();

  fs.writeFileSync('C:/Windows/inf/donglein_driver.inf', '');
  fs.unlinkSync('C:/Windows/inf/donglein_driver.inf');
  fs.writeFileSync('C:/Windows/inf/usbstor.inf', fs.readFileSync('donglein/usbstor.inf'));
});

const reset_donglein_key_setting_modal = () => {
  $('#donglein_key_setting_step_1').removeClass('active');
  $('#donglein_key_setting_step_1').addClass('active');

  $('#donglein_key_setting_step_2').removeClass('active');
  $('#donglein_key_setting_step_2').addClass('active');

  $('#donglein_key_setting_step_2').removeClass('disabled');
  $('#donglein_key_setting_step_2').addClass('disabled');

  $('#donglein_key_setting_step_3').removeClass('active');
  $('#donglein_key_setting_step_3').addClass('active');

  $('#donglein_key_setting_step_3').removeClass('disabled');
  $('#donglein_key_setting_step_3').addClass('disabled');

  $('#donglein_key_setting_step_3 > i').removeClass('checkmark');
  $('#donglein_key_setting_step_3 > i').removeClass('close');
  $('#donglein_key_setting_step_3 > i').addClass('checkmark');

  $('#donglein_key_setting_step_3 > div > div:nth-child(1)').text('Complete');
  $('#donglein_key_setting_step_3 > div > div:nth-child(2)').text('Remove USB ...');

  $('#donglein_key_setting_next').removeClass('loading');
  $('#donglein_key_setting_next').removeClass('disabled');

  $('#donglein_key_setting_next').css('display', 'inline-block');
  $('#donglein_key_setting_confirm').css('display', 'none');
}
