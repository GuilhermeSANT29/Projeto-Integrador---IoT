function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();

  sheet.appendRow([
    new Date(),
    e.parameter.temperatura,
    e.parameter.umidade,
    e.parameter.rotacao,
    e.parameter.ir,
    e.parameter.b1,
    e.parameter.b2
  ]);

  return ContentService.createTextOutput("OK");
}
