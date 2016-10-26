const addon = require('./addon');

setTimeout(function() {
  addon.set_current_user_name();
}, 2000);
