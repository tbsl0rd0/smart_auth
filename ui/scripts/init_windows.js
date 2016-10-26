var win = nw.Window.get();

$('#minimize_window').on('click', function() {
  win.minimize();
});

$('#close_window').on('click', function() {
  win.close();
});

$('body').on('contextmenu', function() {
  return false;
});
