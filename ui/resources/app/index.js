const {app, ipcMain, BrowserWindow} = require('electron');

let initialization_window;
let main_window;

let authentication_factors;
let settings;

app.on('ready', () => {
  initialization_window = new BrowserWindow({
    width: 350,
    height: 300,
    show: false,
    frame: false,
    transparent: true
  });

  initialization_window.setIgnoreMouseEvents(true);

  initialization_window.loadURL(`file://${__dirname}/initialization_window.html`);

  initialization_window.once('ready-to-show', () => {
    initialization_window.show();
  });

  initialization_window.on('closed', () => {
    initialization_window = null;
  });


  ipcMain.on('set_authentication_factors', (event, argument) => {
    authentication_factors = argument;
    event.returnValue = 'set_authentication_factors';
  });

  ipcMain.on('set_settings', (event, argument) => {
    settings = argument;
    event.returnValue = 'set_settings';
  });

  ipcMain.on('get_authentication_factors', (event, argument) => {
    event.returnValue = authentication_factors;
  });

  ipcMain.on('get_settings', (event, argument) => {
    event.returnValue = settings;
  });

  ipcMain.on('command_to_main_process', (event, argument) => {
    if (argument == 'open_main_window') {
      main_window = new BrowserWindow({
        width: 800,
        height: 500,
        show: false,
        frame: false
      });

      main_window.loadURL(`file://${__dirname}/main_window.html`);

      main_window.once('ready-to-show', () => {
        main_window.show();
        initialization_window.close();
      });

      main_window.on('closed', () => {
        main_window = null;
      });
    }
    else if (argument == 'minimize_window') {
      main_window.minimize();
    }
    else if (argument == 'close_window') {
      main_window.close();
    }
  });
});

app.on('window-all-closed', () => {
  app.quit();
});
