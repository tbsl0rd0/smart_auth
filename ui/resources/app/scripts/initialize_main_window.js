const addon = require('./addon');
const {ipcRenderer} = require('electron');

let authentication_factors = ipcRenderer.sendSync('get_authentication_factors', 'get_authentication_factors');
const settings = ipcRenderer.sendSync('get_settings', 'get_settings');

const authentication_factors_enumeration = {'Donglein': 0, 'GoogleOTP': 1, 'SmartIDCard': 2, 'HardwareAuth': 3};
const settings_enumeration = {'on_off': 0, 'hide_other_users_logon_tiles': 1, 'prohibit_fallback_credential_provider': 2};

const donglein_item_html = '' +
'<div class="step" style="cursor: move;" value="Donglein">' +
  '<i class="usb icon"></i>' +
  '<div class="content">' +
    '<div class="title">' +
      'Donglein' +
    '</div>' +
    '<div class="description">' +
      'Need a nomal USB memory storage' +
    '</div>' +
  '</div>' +
'</div>';

const google_otp_item_html = '' +
'<div class="step" style="cursor: move;" value="GoogleOTP">' +
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

const smart_id_card_item_html = '' +
'<div class="step" style="cursor: move;" value="SmartIDCard">' +
  '<i class="icon credit card"></i>' +
  '<div class="content">' +
    '<div class="title">' +
      'Smart ID Card' +
    '</div>' +
    '<div class="description">' +
      'Need a smart ID card' +
    '</div>' +
  '</div>' +
'</div>';

const item_htmls = {'Donglein': donglein_item_html, 'GoogleOTP': google_otp_item_html, 'SmartIDCard': smart_id_card_item_html};

$('#minimize_window').on('click', () => {
  ipcRenderer.send('command_to_main_process', 'minimize_window');
});

$('#close_window').on('click', () => {
  ipcRenderer.send('command_to_main_process', 'close_window');
});
