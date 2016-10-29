var google_otp_key;
var element = document.getElementById('current_auth_stack_items_2');
new Sortable(element, {
  animation: 200,
  group: 'group',
  draggable: ".item",
  onAdd: function(event) {
    var item = event.item;
    if (item.getAttribute("value") == 'GoogleOTP') {
      $('#google_otp_settings_modal').modal('setting', 'closable', false).modal('show');
      var exec = require('child_process').exec;
      exec("google_otp_key_generator.exe", function(error, stdout, stderr) {
        $('#google_otp_key_segment').text(stdout);
        google_otp_key = stdout;
      });
    }
    else {
      registry_key.set(item.getAttribute("value"), 'REG_SZ', '1', function(error) {});
    }
    setTimeout(function() {
      var is_donglein = false;
      var is_smart_id_card = false;
      registry_key.values(function(error, items) {
        for (var key in items) {
          if (items[key].name == 'Donglein' && items[key].value == '1') {
            is_donglein = true;
          }
          if (items[key].name == 'SmartIDCard' && items[key].value == '1') {
            is_smart_id_card = true;
          }
        }
      });
      setTimeout(function() {
        if (is_donglein || is_smart_id_card) {
          registry_key.set('HardwareAuth', 'REG_SZ', '1', function(error) {});
        }
      }, 200);
    }, 200);
  },
  onRemove: function(event) {
    var item = event.item;
    registry_key.set(item.getAttribute("value"), 'REG_SZ', '0', function(error) {});
    setTimeout(function() {
      var is_donglein = false;
      var is_smart_id_card = false;
      registry_key.values(function(error, items) {
        for (var key in items) {
          if (items[key].name == 'Donglein' && items[key].value == '1') {
            is_donglein = true;
          }
          if (items[key].name == 'SmartIDCard' && items[key].value == '1') {
            is_smart_id_card = true;
          }
        }
      });
      setTimeout(function() {
        if (is_donglein == false && is_smart_id_card == false) {
          registry_key.set('HardwareAuth', 'REG_SZ', '0', function(error) {});
        }
      }, 200);
    }, 200);
  }
});

var element_2 = document.getElementById('available_auth_stack_items');
new Sortable(element_2, {
  animation: 200,
  group: 'group'
});

var set_items = function() {
  registry_key.values(function(error, items) {
    for (var key in initial_items) {
      for (var key_2 in items) {
        if (initial_items[key].name != items[key_2].name) {
          continue;
        }
        if (initial_items[key].name == 'GoogleOTP') {
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
        }
        else if (initial_items[key].name == 'SmartIDCard') {
          var html = '' +
          '<div class="step item" style="cursor: move;" value="smartIDCard">' +
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
        }
        else if (initial_items[key].name == 'Donglein') {
          var html = '' +
          '<div class="step item" style="cursor: move;" value="Donglein">' +
            '<i class="usb icon"></i>' +
            '<div class="content">' +
              '<div class="title">' +
                'Dongle In' +
              '</div>' +
              '<div class="description">' +
                'Need a nomal USB memory storage' +
              '</div>' +
            '</div>' +
          '</div>';
        }

        if (items[key_2].value == '0') {
          $('#available_auth_stack_items').prepend(html);
        }
        else if (items[key_2].value == '1') {
          $('#current_auth_stack_items').prepend(html);
          $('#current_auth_stack_items_2').prepend(html);
        }
      }
    }
  });
}
setTimeout(set_items, 200);
