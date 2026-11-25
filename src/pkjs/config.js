// Clay-Konfigurationsseite. Nutzersichtbare Texte englisch.
// messageKey muss Zeichen fuer Zeichen zu package.json passen.
module.exports = [
  {
    "type": "heading",
    "defaultValue": "Nexon"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Dial"
      },
      {
        "type": "select",
        "messageKey": "SCHEME",
        "label": "Colour",
        "defaultValue": "0",
        "options": [
          { "label": "Turquoise", "value": "0" },
          { "label": "White with navy ring", "value": "1" },
          { "label": "Navy", "value": "2" }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "SECONDS",
        "label": "Second hand",
        "description": "Redraws every second and uses more battery.",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];
