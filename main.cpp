#include <iostream>
#include "arduinoEmul.hpp"
#include "toggle.hpp"
#include "sonde.hpp"
#include "lcd.hpp"
#include "string.h"
#include <unistd.h> 

using namespace std;

// prototype definition
void etablirModeEteint();
void etablirModeActif();
void etablirModeAutomatique();
void etablirModeConsigne();
void etablirModeDetailConsigne();
void etablirModeHysteresis();
void etablirModeDetailHysteresis();
void etablirModeVitesse();
void etablirModeNettoyageFiltre();
void etablirModeErreur();
void desactiveTousRelais();
void afficheEtatsGainable();
void vitesseGainable();
void controleTemperature();
void gainable();


// les leds
const int ledCa =  7;  // pin Led fonction CANICULE
const int ledFr =  8;  // pin led fonction FROID
const int ledCh =  9;  // pin Led fonction CHAUFFAGE

// les relais
const int relaisComp = 5;            // pin relais Compresseur
const int relaisV4V =  26;           // pin relais vanne 4 voies
const int relaisVentUniteExt =  13;  // pin relais Ventilateur Unité Exterieur
const int relaisVentExt =  16;       // pin relais Ventilateur Exterieur (petite et grande Vitesses)
const int relaisVentUniteInt =  19;  // pin relais Ventilateur Unité Interieur
const int relaisVentInt =  20;       // pin relais Ventilateur Interieur (petite et grande Vitesses)
const int relaisVentIntPv =  2;     // pin relais Ventilatuer Interieur Petite vitesse
const int relaisVentIntGv =  6;     // pin relais Ventilateur Interieur Grande vitesse

// les capteurs
const int thermCh1Pin =  8;        // pin thermostat Chambre 1 , bouton poussoir pour test
const int thermCh2Pin =  9;        // pin thermostat Chambre 2 , ""
const int thermCh3Pin =  10;       // pin thermostat Chambre 3 , ""
const int thermCh4Pin =  11;       // pin thermostat Chambre 4 , ""
const int thermSalonPin =  12;     // pin thermostat Salon , ""
const int capteurFiltrePin =  40;  // pin capteur de presence porte filtre
Toggle thermCh1;
Toggle thermCh2;
Toggle thermCh3;
Toggle thermCh4;
Toggle thermSalon;
Toggle capteurFiltre;
const unsigned int ms = 200;

// les boutons
const int menuPin =  22;         // pin bouton Menu
const int validPin =  23;        // pin bouton Valid
const int boutonPlusPin =  24;   // pin bouton Plus
const int boutonMoinsPin =  25;  // pin bouton Moins
Toggle boutonMenu;
Toggle boutonValid;
Toggle boutonPlus;
Toggle boutonMoins;
const unsigned int ms1 = 2000;

// les led boutons
const int ledBoutonMenu =  27;  // pin Led bouton Menu
const int ledBoutonValid =  21; // pin led bouton Valid
const int ledBoutonPlus =  17;  // pin led bouton Plus
const int ledBoutonMoins =  18; // pin led bouton Moins


// Les sondes
Sonde sondes[] = {
  Sonde(2,(char *) "S-Ext:"),  // pin sonde Exterieur
  Sonde(3,(char *) "U-Ext:"),  // pin sonde Unite Exterieur
  { 4, "E-Ext:" },  // pin sonde Echangeur Exterieur
  { 5, "U-Int:" },  // pin sonde Unite Interieur
  { 6, "E-Int:" }   // pin sonde Echangeur Interieur
};
constexpr size_t nombreDeSondes = sizeof sondes / sizeof * sondes;
size_t indexSondeEnCours = 0;
float temperatureAffichee = -127;

enum : int { SondeExterieur,
              UniteExterieur,
              EchangeurExterieur,
              UniteInterieur,
              EchangeurInterieur
            };
#define tempExtLue (sondes[SondeExterieur].temperature())
#define tempUnitExtLue (sondes[UniteExterieur].temperature())
#define tempEchangeurExtLue (sondes[EchangeurExterieur].temperature())
#define tempUnitIntLue (sondes[UniteInterieur].temperature())
#define tempEchangeurIntLue (sondes[EchangeurInterieur].temperature())

// les tempos
unsigned long tempoTempLcd;
unsigned long tempoLcdConsigne;
unsigned long tempoBacklightLcd;

unsigned long tempoV4VFr = 0;
unsigned long tempoCompFr = 0;
unsigned long tempoCompCh = 0;
unsigned long tempoV4VCh = 0;
unsigned long tempoDegCh = 0;
unsigned long tempoDegNat = 0;
unsigned long tempoFinDegNat = 0;
unsigned long tempoEgouttageFr = 0;
unsigned long tempoEgouttageNat = 0;
unsigned long tempoEgouttageEle = 0;
unsigned long tempoFinEgouttageEle = 0;

unsigned long departChronoFiltre;
unsigned long finChronoFiltre;
unsigned long cumulTempsFiltre;
unsigned long nettoyageFiltre;

unsigned long tempoLedFrClignoteDeg = 0;
bool ledFrCl = 0;
unsigned long tempoLedChClignoteDeg = 0;
bool ledChCl = 0;
unsigned long tempoLedCaClignoteDeg = 0;
bool ledCaCl = 0;
unsigned long tempoLedArretCompletProgramClignote = 0;
bool ledChFrCaCl = 0;
unsigned long tempoLedBoutonClignote = 0;
bool ledBoutonMenuCl = 0;
bool ledBoutonMoinsCl = 0;
bool ledBoutonPlusCl = 0;
bool ledBoutonValidCl = 0;

const unsigned long AutoTemp = 30000ul;
static unsigned long chronoAutoTemp = -AutoTemp;

// les consignes
struct Consigne {
  const char* nom;
  float consigne;
};

struct Consigne consignes[] = {
  { "Temp-Ext", 13.5 },    //
  { "T-Canicule", 30.0 },  //
  { "GV-Ext-Fr", 20.0 },   //
  { "Bloc-Ch", 12.0 },     //
  { "GV-Ext-Ch", 5.0 },    //
  { "Deg-Na-Ch", 5.0 },    //
  { "Deg-Ele-Ch", -3.0 },  //
  { "Fin-Deg-Ch", 12.5 },  //
  { "PV-Int-Fr", 25.0 },   //
  { "PV-Int-Ch", 20.0 },   //
  { "D-V-Int-Ch", 30.0 },  //
  { "Deg-Fr", -1.0 },      //
  { "Fin-Deg-Fr", 15.0 },  //
  { "Temp-Delta", 6 }      //
};
constexpr size_t nombreDeConsignes = sizeof consignes / sizeof * consignes;
size_t indexConsigneEnCours = 0;

float consigneIntCa;

// les hysteresis
struct Hysteresi {
  const char* nom;
  float hysteresis;
};

struct Hysteresi hysteresis[] = {
  { "hyst-Ext", 1.0 },
  { "hyst-U-Ext", 0.5 },
  { "hyst-U-Int", 0.5 },
  { "hyst-Ca", 0.5 }
};
constexpr size_t nombreDeHysteresis = sizeof hysteresis / sizeof * hysteresis;
size_t indexHysteresiEnCours = 0;

bool tempIntCa = false;
bool tempExt = false;
bool tempVentIntCh = false;
bool tempVentExtCh = false;
bool tempVentIntFr = false;
bool tempVentExtFr = false;

// les vitesses interieur
struct Vitesse {
  const char* nom;
  int vitesse;
};

struct Vitesse vitesses[] = {
  { "V-Auto", 0 },
  { "P-Vitesse", 1 },
  { "P-Vitesse", 2 },
  { "G-Vitesse", 3 },
  { "G-Vitesse", 4 }
};
constexpr size_t nombreDeVitesses = sizeof vitesses / sizeof * vitesses;
size_t indexVitesseEnCours = 0;
bool modifVitesseInt = false;
int vitesseTravail = vitesses[indexVitesseEnCours].vitesse;

bool departGainable = false; // active ou desactive la machine a etasGainable (arret)

// le lcd
const uint8_t nbColonnes = 16;
const uint8_t nbLignes = 2;
hd44780_I2Cexp lcd;

// les caracteres speciaux
unsigned char flecheFr[8] = {
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb10101,
  (unsigned char ) 0xb01110,
  (unsigned char ) 0xb00100
};

unsigned char  flecheCh[8] = {
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb01110,
  (unsigned char ) 0xb10101,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100
};

unsigned char goutte[8] = {
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb01010,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb01110
};

unsigned char degivrage[8] = {
  (unsigned char ) 0xb11111,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb01010,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb10001,
  (unsigned char ) 0xb01110
};

unsigned char  eclaire[8] = {
  (unsigned char ) 0xb00010,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb01000,
  (unsigned char ) 0xb11111,
  (unsigned char ) 0xb00010,
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb01000,
  (unsigned char ) 0xb10000
};

unsigned char  soleil[8] = {
  (unsigned char ) 0xb00100,
  (unsigned char ) 0xb10101,
  (unsigned char ) 0xb01110,
  (unsigned char ) 0xb11010,
  (unsigned char ) 0xb01011,
  (unsigned char ) 0xb01110,
  (unsigned char ) 0xb10101,
  (unsigned char ) 0xb00100
};

unsigned char  ventilateur[8] = {
  (unsigned char ) 0xb01100,
  (unsigned char ) 0xb01100,
  (unsigned char ) 0xb00101,
  (unsigned char ) 0xb11011,
  (unsigned char ) 0xb11011,
  (unsigned char ) 0xb10100,
  (unsigned char ) 0xb00110,
  (unsigned char ) 0xb00110
};

// les fonctions de l'affichages
void affiche(const char* etiquette, float valeur, int ligne, int precision = 2, int tailleValeur = 6) {
  // on a nbColonnes-tailleValeur cases pour le nom - cadré à gauche
  // tailleValeur cases pour la consigne - cadrée à droite
  char bufferEtiquette[nbColonnes - tailleValeur + 1];
  strncpy(bufferEtiquette, etiquette, sizeof bufferEtiquette);  // ça va tronquer l'étiquette si nécesaire
  lcd.setCursor(0, ligne);
  lcd.print(bufferEtiquette);

  char bufferValeur[nbColonnes];                           // suffisamment grand  pour nos besoins
//  dtostrf(valeur, tailleValeur, precision, bufferValeur);  // conversion en chaîne de caractères
  char formatBuffer[64];
  sprintf(formatBuffer,"%%%d.%df",tailleValeur,precision);
  sprintf(bufferValeur,formatBuffer,valeur);

  int nbEspaces = nbColonnes - (strlen(bufferEtiquette) + strlen(bufferValeur));
  for (int i = 0; i < nbEspaces; i++) lcd.write(' ');  // pour cadrer la valeur à droite

  lcd.print(bufferValeur);
//  static uint32_t x = 0;
}

void affiche(const char* etiquette, int valeur, int ligne, int tailleValeur = 6) {
  // on a nbColonnes-tailleValeur cases pour le nom - cadré à gauche
  // tailleValeur cases pour la consigne - cadrée à droite
  char bufferEtiquette[nbColonnes - tailleValeur + 1];
  strncpy(bufferEtiquette, etiquette, sizeof bufferEtiquette);  // ça va tronquer l'étiquette si nécesaire
  lcd.setCursor(0, ligne);
  lcd.print(bufferEtiquette);

  char bufferValeur[nbColonnes];   // suffisamment grand  pour nos besoins
  sprintf(bufferValeur,"%d",valeur);
  //,itoa(valeur, bufferValeur, 10);  // conversion en chaîne de caractères

  int nbEspaces = nbColonnes - (strlen(bufferEtiquette) + strlen(bufferValeur));
  for (int i = 0; i < nbEspaces; i++) lcd.write(' ');  // pour cadrer la valeur à droite

  lcd.print(bufferValeur);
}

void affiche(const char* etiquette, int ligne) {

  char bufferEtiquette[nbColonnes];
  strncpy(bufferEtiquette, etiquette, sizeof bufferEtiquette);
  lcd.setCursor(0, ligne);
  lcd.print(bufferEtiquette);
}

// les consignes en fonction des sondes
void consigneSonde() { // affichage consignes par rapport au sondes sur la ligne 0
  if (indexConsigneEnCours == 0 || indexConsigneEnCours == 1) { // si : consigne 0 (13.5°C) ou consigne 1 (30.0°C)
    affiche(consignes[indexConsigneEnCours].nom, sondes[0].temperature(), 0); // affiche la consigne en cours et affiche la temperature lue par la sonde 0 (Temp-Ext) a la ligne 0
  } else if (indexConsigneEnCours == 2 || indexConsigneEnCours == 3 || indexConsigneEnCours == 4 || indexConsigneEnCours == 5) {
    affiche(consignes[indexConsigneEnCours].nom, sondes[1].temperature(), 0);
  } else if (indexConsigneEnCours == 6 || indexConsigneEnCours == 7) {
    affiche(consignes[indexConsigneEnCours].nom, sondes[2].temperature(), 0);
  } else if (indexConsigneEnCours == 8 || indexConsigneEnCours == 9) {
    affiche(consignes[indexConsigneEnCours].nom, sondes[3].temperature(), 0);
  } else if (indexConsigneEnCours == 10 || indexConsigneEnCours == 11 || indexConsigneEnCours == 12) {
    affiche(consignes[indexConsigneEnCours].nom, sondes[4].temperature(), 0);
  } else { // si non :
    affiche(consignes[indexConsigneEnCours].nom, consigneIntCa, 0); // affiche la consigne en cours et affiche la consigne canicule
  }
}

// les modification de vitesses interieur
void vitesseInterieur() { // modification des relais ventilateur interieur en fonction de l'affichage pour changer les vitesses
  if (indexVitesseEnCours == 0) { // si : 0 (automatique)
    modifVitesseInt = false; // pas de modification de vitesse
    vitesseTravail = vitesses[indexVitesseEnCours].vitesse;
  } else if (indexVitesseEnCours == 1) { // si non si : 1 (petite vitesse 1)
    modifVitesseInt = true; // modification de vitesse possible
    digitalWrite(relaisVentInt, LOW); // desactive relais ventilateur interieur petite vitesse 1 et grande vitesse 4
    digitalWrite(relaisVentIntPv, LOW); // desactive relais ventilateur interieur petite vitesse 2
    digitalWrite(relaisVentIntGv, LOW); // desactive relais ventilateur interieur grande vitesse 3
    vitesseTravail = vitesses[indexVitesseEnCours].vitesse;
  } else if (indexVitesseEnCours == 2) { // si non si ; 2 (petite vitesse 2)
    modifVitesseInt = true; // modification de vitesse possible
    digitalWrite(relaisVentInt, LOW); // desactive relais ventilateur interieur petite vitesse 1 et grande vitesse 4
    digitalWrite(relaisVentIntPv, HIGH); // active relais ventilateur interieur petite vitesse 2
    digitalWrite(relaisVentIntGv, LOW); // desactive relais ventilateur interieur grande vitesse 3
    vitesseTravail = vitesses[indexVitesseEnCours].vitesse;
  } else if (indexVitesseEnCours == 3) {
    modifVitesseInt = true;
    digitalWrite(relaisVentInt, LOW);
    digitalWrite(relaisVentIntPv, LOW);
    digitalWrite(relaisVentIntGv, HIGH);
    vitesseTravail = vitesses[indexVitesseEnCours].vitesse;
  } else if (indexVitesseEnCours == 4) {
    modifVitesseInt = true;
    digitalWrite(relaisVentInt, HIGH);
    digitalWrite(relaisVentIntPv, LOW);
    digitalWrite(relaisVentIntGv, LOW);
    vitesseTravail = vitesses[indexVitesseEnCours].vitesse;
  }
}

// les erreurs
bool erreur = false;

// les erreurs sondes
void erreurs() {
  if (tempExtLue == -127) {
    lcd.backlight();
    lcd.clear();
    affiche("!Er-Sonde-Ext!", 1);
    etablirModeErreur();
  } else if (tempUnitExtLue == -127) {
    lcd.backlight();
    lcd.clear();
    affiche("!Er-Sonde-UExt!", 1);
    etablirModeErreur();
  } else if (tempEchangeurExtLue == -127) {
    lcd.backlight();
    lcd.clear();
    affiche("!Er-Sonde-EExt!", 1);
    etablirModeErreur();
  } else if (tempUnitIntLue == -127) {
    lcd.backlight();
    lcd.clear();
    affiche("!Er-Sonde-UInt!", 1);
    etablirModeErreur();
  } else if (tempEchangeurIntLue == -127) {
    lcd.backlight();
    lcd.clear();
    affiche("!Er-Sonde-EInt!", 1);
    etablirModeErreur();
  }
}

// La machine à état de commande
enum { ETEINT,
       CONSIGNE,
       DETAIL_CONSIGNE,
       HYSTERESIS,
       DETAIL_HYSTERESIS,
       VITESSE,
       ACTIF,
       AUTOMATIQUE,
       ERREUR,
       NETTOYAGE_FILTRE
     } mode = ETEINT;

// les Variables de commande
bool entretienFiltre = false; // active ou desactive
int ledMaitre = 0;

// les fonctions de commande
void etablirModeEteint() {
  lcd.backlight();
  if (departGainable == true) {
    lcd.clear();
    lcd.print("** ETEINDRE ? **");
  } else {
    lcd.clear();
    lcd.print("***  ETEINT  ***");
  }
  mode = ETEINT;
}

void etablirModeActif() {
  lcd.backlight();
  lcd.clear();
  lcd.print("** DEMARAGE ? **");
  mode = ACTIF;
}

void etablirModeAutomatique() {
  lcd.backlight();
  lcd.clear();
  departGainable = true;
  tempoBacklightLcd = millis();
  mode = AUTOMATIQUE;
}

void etablirModeConsigne() {
  lcd.backlight();
  lcd.clear();
  lcd.print("*** CONSIGNE ***");
  affiche(consignes[indexConsigneEnCours].nom, consignes[indexConsigneEnCours].consigne, 1);
  tempoLcdConsigne = millis();
  mode = CONSIGNE;
}

void etablirModeDetailConsigne() {
  lcd.backlight();
  lcd.clear();
  consigneSonde();
  affiche("consigne:", consignes[indexConsigneEnCours].consigne, 1);
  tempoLcdConsigne = millis();
  mode = DETAIL_CONSIGNE;
}

void etablirModeHysteresis() {
  lcd.backlight();
  lcd.clear();
  lcd.print("** HYSTERESIS **");
  affiche(hysteresis[indexHysteresiEnCours].nom, hysteresis[indexHysteresiEnCours].hysteresis, 1);
  mode = HYSTERESIS;
}

void etablirModeDetailHysteresis() {
  lcd.backlight();
  lcd.clear();
  affiche(hysteresis[indexHysteresiEnCours].nom, hysteresis[indexHysteresiEnCours].hysteresis, 0);
  affiche("hysteresis:", hysteresis[indexHysteresiEnCours].hysteresis, 1);
  tempoLcdConsigne = millis();
  mode = DETAIL_HYSTERESIS;
}

void etablirModeVitesse() {
  lcd.backlight();
  lcd.clear();
  lcd.print("*** VITESSES ***");
  affiche(vitesses[indexVitesseEnCours].nom, vitesses[indexVitesseEnCours].vitesse, 1);
  mode = VITESSE;
}

void etablirModeNettoyageFiltre() {
  lcd.backlight();
  lcd.clear();
  mode = NETTOYAGE_FILTRE;
}

void etablirModeErreur() {
  affiche(" **  ERREUR  **", 0);
  departGainable = false;
  mode = ERREUR;
}

void majBoutons() {
  boutonMenu.poll();
  boutonValid.poll();
  boutonPlus.poll();
  boutonMoins.poll();
}

void majCapteurs() {
  thermCh1.poll();
  thermCh2.poll();
  thermCh3.poll();
  thermCh4.poll();
  thermSalon.poll();
  capteurFiltre.poll();
}

// les leds clignotantes des boutons
void activeLedBoutonClignote() { // clignotement des leds des boutons a la meme cadence
  if (millis() - tempoLedBoutonClignote > 300) {
    ledMaitre = !ledMaitre;
    if (mode == ETEINT && departGainable == true)
    {
      digitalWrite(ledBoutonMenu, ledMaitre);
      digitalWrite(ledBoutonValid, ledMaitre);
      digitalWrite(ledBoutonPlus, LOW);
      digitalWrite(ledBoutonMoins, LOW);
    } else if (mode == ETEINT) {
      digitalWrite(ledBoutonMenu, ledMaitre);
      digitalWrite(ledBoutonValid, LOW);
      digitalWrite(ledBoutonPlus, LOW);
      digitalWrite(ledBoutonMoins, LOW);
    } else if (mode == CONSIGNE || mode == HYSTERESIS || mode == VITESSE || mode == ERREUR) {
      digitalWrite(ledBoutonMenu, ledMaitre);
      digitalWrite(ledBoutonValid, ledMaitre);
      digitalWrite(ledBoutonMoins, ledMaitre);
      digitalWrite(ledBoutonPlus, ledMaitre);
    } else if (mode == DETAIL_CONSIGNE || mode == DETAIL_HYSTERESIS) {
      digitalWrite(ledBoutonValid, ledMaitre);
      digitalWrite(ledBoutonMoins, ledMaitre);
      digitalWrite(ledBoutonPlus, ledMaitre);
      digitalWrite(ledBoutonMenu, LOW);
    } else if (mode == ACTIF) {
      digitalWrite(ledBoutonMenu, ledMaitre);
      digitalWrite(ledBoutonValid, ledMaitre);
      digitalWrite(ledBoutonPlus, LOW);
      digitalWrite(ledBoutonMoins, LOW);
    } else if (mode == AUTOMATIQUE) {
      digitalWrite(ledBoutonMenu, ledMaitre);
      digitalWrite(ledBoutonValid, LOW);
      digitalWrite(ledBoutonPlus, LOW);
      digitalWrite(ledBoutonMoins, LOW);
    }
    tempoLedBoutonClignote = millis();
  }
}

// le mode
void gestionEtat() {

  majBoutons();
  gainable();

  switch (mode) {

    case ETEINT:

      if (departGainable == true) {
        if (millis() - tempoLcdConsigne >= 15000) {
          etablirModeAutomatique();
          tempoBacklightLcd = millis();
        }
        if (boutonMenu.onPress()) {
          etablirModeAutomatique();
        } else if (boutonValid.onPress()) {
          departGainable = false;
          etablirModeEteint();
        } else {
          activeLedBoutonClignote();
        }
      } else if (boutonMenu.onPress() && !capteurFiltre.pressedFor(ms)) {
        etablirModeActif();
      } else {
        activeLedBoutonClignote();
      }
      break;

    case CONSIGNE:

      if (millis() - tempoLcdConsigne >= 15000) {
        etablirModeAutomatique();
        tempoBacklightLcd = millis();
      }
      if (boutonPlus.onPress()) {
        tempoLcdConsigne = millis();
        indexConsigneEnCours = (indexConsigneEnCours + 1) % nombreDeConsignes;
        affiche(consignes[indexConsigneEnCours].nom, consignes[indexConsigneEnCours].consigne, 1);
      } else if (boutonMoins.onPress()) {
        tempoLcdConsigne = millis();
        if (indexConsigneEnCours == 0) indexConsigneEnCours = nombreDeConsignes - 1;
        else indexConsigneEnCours--;
        affiche(consignes[indexConsigneEnCours].nom, consignes[indexConsigneEnCours].consigne, 1);
      } else if (boutonValid.onPress()) {
        etablirModeDetailConsigne();
      } else if (boutonMenu.onPress()) {
        etablirModeHysteresis();
      } else {
        activeLedBoutonClignote();
      }
      break;

    case DETAIL_CONSIGNE:

      {
        if (millis() - tempoLcdConsigne >= 15000) {
          etablirModeAutomatique();
          tempoBacklightLcd = millis();
        }
        consigneSonde();
        if (boutonPlus.onPress()) {
          tempoLcdConsigne = millis();
          consignes[indexConsigneEnCours].consigne += 0.5;
          affiche("consigne:", consignes[indexConsigneEnCours].consigne, 1);
        } else if (boutonMoins.onPress()) {
          tempoLcdConsigne = millis();
          consignes[indexConsigneEnCours].consigne -= 0.5;
          affiche("consigne:", consignes[indexConsigneEnCours].consigne, 1);
        } else if (boutonValid.onPress()) {
          etablirModeConsigne();
        } else {
          activeLedBoutonClignote();
        }
      }
      break;

    case HYSTERESIS:

      if (millis() - tempoLcdConsigne >= 15000) {
        etablirModeAutomatique();
        tempoBacklightLcd = millis();
      }
      if (boutonPlus.onPress()) {
        tempoLcdConsigne = millis();
        indexHysteresiEnCours = (indexHysteresiEnCours + 1) % nombreDeHysteresis;
        affiche(hysteresis[indexHysteresiEnCours].nom, hysteresis[indexHysteresiEnCours].hysteresis, 1);
      } else if (boutonMoins.onPress()) {
        tempoLcdConsigne = millis();
        if (indexHysteresiEnCours == 0) indexHysteresiEnCours = nombreDeHysteresis - 1;
        else indexHysteresiEnCours--;
        affiche(hysteresis[indexHysteresiEnCours].nom, hysteresis[indexHysteresiEnCours].hysteresis, 1);
      } else if (boutonValid.onPress()) {
        etablirModeDetailHysteresis();
      } else if (boutonMenu.onPress()) {
        vitesseGainable();
      } else {
        activeLedBoutonClignote();
      }
    case DETAIL_HYSTERESIS:

      if (millis() - tempoLcdConsigne >= 15000) {
        etablirModeAutomatique();
        tempoBacklightLcd = millis();
      }
      if (boutonPlus.onPress()) {
        tempoLcdConsigne = millis();
        hysteresis[indexHysteresiEnCours].hysteresis += 0.5;
        affiche("hysteresis:", hysteresis[indexHysteresiEnCours].hysteresis, 1);
      } else if (boutonMoins.onPress()) {
        tempoLcdConsigne = millis();
        hysteresis[indexHysteresiEnCours].hysteresis -= 0.5;
        affiche("hysteresis:", hysteresis[indexHysteresiEnCours].hysteresis, 1);
      } else if (boutonValid.onPress()) {
        etablirModeHysteresis();
      } else {
        activeLedBoutonClignote();
      }

      break;

    case VITESSE:

      {
        if (millis() - tempoLcdConsigne >= 15000) {
          etablirModeAutomatique();
          tempoBacklightLcd = millis();
        }
        vitesseInterieur();
        if (boutonPlus.onPress()) {
          tempoLcdConsigne = millis();
          indexVitesseEnCours = (indexVitesseEnCours + 1) % nombreDeVitesses;
          affiche(vitesses[indexVitesseEnCours].nom, vitesses[indexVitesseEnCours].vitesse, 1);
        } else if (boutonMoins.onPress()) {
          tempoLcdConsigne = millis();
          if (indexVitesseEnCours == 0) indexVitesseEnCours = nombreDeVitesses - 1;
          else indexVitesseEnCours--;
          affiche(vitesses[indexVitesseEnCours].nom, vitesses[indexVitesseEnCours].vitesse, 1);
        } else if (boutonMenu.onPress()) {
          etablirModeEteint();
        } else if (boutonValid.onPress()) {
          etablirModeAutomatique();
        } else {
          tempoBacklightLcd = millis();
          activeLedBoutonClignote();
        }
      }
      break;

    case ACTIF:

      if (boutonValid.onPress()) {
        etablirModeAutomatique();
        tempoBacklightLcd = millis();
      } else if (boutonMenu.onPress()) {
        digitalWrite(ledBoutonValid, LOW);
        etablirModeEteint();
      } else {
        activeLedBoutonClignote();
      }
      break;

    case AUTOMATIQUE:

      {
        const unsigned long Auto = 10000ul;
        static unsigned long chronoAuto = -Auto;
        bool noBacklightLcd = false;

        if (entretienFiltre == true) {
          etablirModeNettoyageFiltre();
        } else if (millis() - chronoAuto >= Auto) {
          indexSondeEnCours = (indexSondeEnCours + 1) % nombreDeSondes;
          chronoAuto = millis();
        } else {
          afficheEtatsGainable();
          affiche(sondes[indexSondeEnCours].nom, sondes[indexSondeEnCours].temperature(), 1);
        }
        if (millis() - tempoBacklightLcd >= 60000) {
          digitalWrite(ledBoutonValid, LOW);
          digitalWrite(ledBoutonPlus, LOW);
          digitalWrite(ledBoutonMoins, LOW);
          digitalWrite(ledBoutonMenu, LOW);
          lcd.noBacklight();
          noBacklightLcd = true;
          if (boutonMenu.onPress()) {
            lcd.backlight();
            tempoBacklightLcd = millis();
            noBacklightLcd = false;
          }
        } else if (boutonMenu.onPress() && noBacklightLcd == false) {
          etablirModeConsigne();
        } else {
          activeLedBoutonClignote();
          erreurs();
        }
      }
      break;

    case ERREUR:

      if (boutonMenu.onPress()) {
        erreurs();
      } else if (boutonValid.onPress()) {
        etablirModeEteint();
      } else {
        activeLedBoutonClignote();
      }
      break;

    case NETTOYAGE_FILTRE:

      lcd.backlight();
      affiche("Nettoyage       ", 0);
      affiche("         Filtre", 1);

      break;
  }
}

// la machine à état gainable
enum { ARRET,
       DEPART,
       COMMANDE_FROID,
       TEMPO_V4V,
       TEMPO_COMPRESSEUR_FROID,
       COMPRESSEUR_FROID,
       DEGIVRAGE_FROID,
       EGOUTTAGE_FROID,
       COMMANDE_CHAUFFAGE,
       TEMPO_COMPRESSEUR_CHAUFFAGE,
       TEMPO_DEGIVRAGE,
       MODE_DEGIVRAGE,
       DEGIVRAGE_NATUREL,
       EGOUTTAGE_NATUREL,
       TEMPO_DEG_V4V,
       TEMPO_DEG_COMPRESSEUR,
       DEGIVRAGE_ELECTRIC,
       EGOUTTAGE_CHAUFFAGE,
       FIN_EGOUTTAGE_CHAUFFAGE,
       COMMANDE_CANICULE,
       TEMPO_V4V_CANICULE,
       TEMPO_COMPRESSEUR_CANICULE,
       COMPRESSEUR_CANICULE,
       FILTRE
     } etatsGainable;

// les variables de gainable
bool fonctionFr = false;
bool fonctionCh = false;
bool fonctionCa = false;
bool caniculeLed = false;

// les commandes des fonctions
void commandeFroid() {
  majCapteurs();
  if (thermSalon.pressedFor(ms) || thermCh1.pressedFor(ms) || thermCh2.pressedFor(ms) || thermCh3.pressedFor(ms) || thermCh4.pressedFor(ms)) {
    fonctionFr = true;
  } else {
    fonctionFr = false;
  }
}

void commandeChauffage() {
  majCapteurs();
  if (tempUnitExtLue >= consignes[3].consigne) {
    fonctionCh = false;
  } else if (!thermSalon.pressedFor(ms) || !thermCh1.pressedFor(ms) || !thermCh2.pressedFor(ms) || !thermCh3.pressedFor(ms) || !thermCh4.pressedFor(ms)) {
    fonctionCh = true;
  } else {
    fonctionCh = false;
  }
}

void commandeCanicule() {
  majCapteurs();
  if (tempIntCa && (thermSalon.pressedFor(ms) || thermCh1.pressedFor(ms) || thermCh2.pressedFor(ms) || thermCh3.pressedFor(ms) || thermCh4.pressedFor(ms))) {
    fonctionCa = true;
  } else {
    fonctionCa = false;
  }
}

// les hysteresis de consignes
void hysteresisTempExt() {
  if (tempExt) {
    tempExt = (tempExtLue <= (consignes[0].consigne + hysteresis[0].hysteresis));
  } else {
    tempExt = (tempExtLue <= (consignes[0].consigne - hysteresis[0].hysteresis));
  }
}

void hysteresisTempIntCa() {
  if (tempIntCa) {
    tempIntCa = (tempUnitIntLue >= (consigneIntCa - hysteresis[3].hysteresis));
  } else {
    tempIntCa = (tempUnitIntLue >= (consigneIntCa + hysteresis[3].hysteresis));
  }
}

void hysteresisTempVentIntCh() {
  if (tempVentIntCh) {
    tempVentIntCh = (tempUnitIntLue >= (consignes[9].consigne - hysteresis[2].hysteresis));
  } else {
    tempVentIntCh = (tempUnitIntLue >= (consignes[9].consigne + hysteresis[2].hysteresis));
  }
}

void hysteresisTempVentExtCh() {
  if (tempVentExtCh) {
    tempVentExtCh = (tempUnitExtLue <= (consignes[4].consigne + hysteresis[1].hysteresis));
  } else {
    tempVentExtCh = (tempUnitExtLue <= (consignes[4].consigne - hysteresis[1].hysteresis));
  }
}

void hysteresisTempVentIntFr() {
  if (tempVentIntFr) {
    tempVentIntFr = (tempUnitIntLue <= (consignes[8].consigne + hysteresis[2].hysteresis));
  } else {
    tempVentIntFr = (tempUnitIntLue <= (consignes[8].consigne - hysteresis[2].hysteresis));
  }
}

void hysteresisTempVentExtFr() {
  if (tempVentExtFr) {
    tempVentExtFr = (tempUnitExtLue >= (consignes[2].consigne - hysteresis[1].hysteresis));
  } else {
    tempVentExtFr = (tempUnitExtLue >= (consignes[2].consigne + hysteresis[1].hysteresis));
  }
}

// les clignotements des leds des fonctions en degivrage
void activeLedDegivrageFr() {
  if (caniculeLed == true) {
    digitalWrite(ledFr, LOW);
  } else {
    if (millis() - tempoLedFrClignoteDeg > 300) {
      tempoLedFrClignoteDeg = millis();
      ledFrCl = (ledFrCl == HIGH) ? LOW : HIGH;
      digitalWrite(ledFr, ledFrCl);
    }
  }
}

void activeLedDegivrageCa() {
  if (caniculeLed == true) {
    if (millis() - tempoLedCaClignoteDeg > 300) {
      tempoLedCaClignoteDeg = millis();
      ledCaCl = (ledCaCl == HIGH) ? LOW : HIGH;
      digitalWrite(ledCa, ledCaCl);
    }
  }
}

void activeLedDegivrageCh() {
  if (millis() - tempoLedChClignoteDeg > 300) {
    tempoLedChClignoteDeg = millis();
    ledChCl = (ledChCl == HIGH) ? LOW : HIGH;
    digitalWrite(ledCh, ledChCl);
  }
}

// les relais des ventilateurs interieur et exterieur
void activeRelaisVentIntFroid() {      // controle ventilateurs en FROID
  if (tempVentIntFr) {                 // si : la temperature unite interieur est inferieur a consigne Petite vitesse interieur 25°C a l'aspiration
    digitalWrite(relaisVentInt, LOW);  // ventilateur interieur petite vitesse
  } else {                             // si non :
    digitalWrite(relaisVentInt, HIGH);  // ventilateur interieur grande vitesse
  }
}

void activeRelaisVentExtFroid() {           // controle ventilateur exterieur en froid
  digitalWrite(relaisVentUniteExt, HIGH);  // relais ventilateur unité exterieur oN
  if (tempVentExtFr) {                     // si : la temperature unite exterieur est superieur a consigne grande vitesse exterieur 20°C ambiante
    digitalWrite(relaisVentExt, HIGH);     // ventilateur exterieur grande vitesse
  } else {                                 // si non :
    digitalWrite(relaisVentExt, LOW);      // ventilateur exterieur petite vitesse
  }
}

void activeRelaisVentIntCh() { // controle ventilateur interieur en chauffage
  if (tempVentIntCh) {                 // si : la temperature unite interieur est superieur a consigne petite vitesse 20°C a l'aspiration
    digitalWrite(relaisVentInt, LOW);  // petit vitesse
  } else {                             // si non :
    digitalWrite(relaisVentInt, HIGH);  // grande vitesse
  }
}

void activeRelaisVentExtCh() {             // controle ventilateur exterieur en chauffage
  digitalWrite(relaisVentUniteExt, HIGH);  // relais ventilateur de l'unité exterieur oN
  if (tempVentExtCh) {                     // si : la temperature unite exterieur est inferieur a 5°C ambiante
    digitalWrite(relaisVentExt, HIGH);     // ventilateur exterieur grande vitesse
  } else {                                 // si non :
    digitalWrite(relaisVentExt, LOW);      // ventilateur exterieur petite vitesse
  }
}

void activeRelaisVentsCanicule() {  // controle ventilateurs Interieur Canicule
  digitalWrite(relaisVentUniteInt, HIGH); // relais ventilateur unite interieur oN (coupure au neutre)
  digitalWrite(relaisVentInt, HIGH);  // relais ventilateur interieur grande vitesse
  digitalWrite(relaisVentUniteExt, HIGH); // relais ventilateur unite exterieur oN (coupure au neutre en serie avec pressostat Hp2)
  digitalWrite(relaisVentExt, HIGH); // relais ventilateur exterieur Grande vitesse
}

void activeRelaisVentIntDegFr() { // controle ventilatuer interieur en degivrage froid
  digitalWrite(relaisVentUniteInt, HIGH);  // relais ventilateur de l'unité interieur oN
  digitalWrite(relaisVentInt, HIGH);       // relais ventilateur interieur grande vitesse oN
}

void activeRelaisVentExtDegCh() { // controle ventilateur exterieur en degivrage chauffage
  digitalWrite(relaisVentUniteExt, HIGH); // relais ventilateur unite exterieur oN (coupure au neutre en serie avec pressostat Hp2)
  digitalWrite(relaisVentExt, HIGH); // relais ventilateur exterieur Grande vitesse
}

// l'arret des relais
void desactiveTousRelais() {
  digitalWrite(relaisVentUniteExt, LOW);
  digitalWrite(relaisVentUniteInt, LOW);
  digitalWrite(relaisVentInt, LOW);
  digitalWrite(relaisVentIntPv, LOW);
  digitalWrite(relaisVentIntGv, LOW);
  digitalWrite(relaisVentExt, LOW);
  digitalWrite(relaisComp, LOW);
  digitalWrite(relaisV4V, LOW);
}

// les affichages des etats du gainable
void afficheEtatsGainable() {
  if (etatsGainable == COMMANDE_FROID) {
    affiche("Froid oFF       ", 0);
    lcd.setCursor(15, 0);
    lcd.print(" ");
  } else if (etatsGainable == TEMPO_V4V || etatsGainable == TEMPO_COMPRESSEUR_FROID || etatsGainable == COMPRESSEUR_FROID) {
    affiche("Froid        ", 0);
    lcd.setCursor(13, 0);
    lcd.print(char(8));
    if (modifVitesseInt == true) {
      lcd.setCursor(14, 0);
      lcd.print(vitesseTravail);
    } else if (digitalRead(relaisVentInt)) {
      lcd.setCursor(14, 0);
      lcd.print("4");
    } else {
      lcd.setCursor(14, 0);
      lcd.print("1");
    }
    if (digitalRead(relaisComp)) {
      lcd.setCursor(15, 0);
      lcd.print(char(1));
    }
  } else if (etatsGainable == DEGIVRAGE_FROID) {
    affiche("Degivrage       ", 0);
    lcd.setCursor(15, 0);
    lcd.print(char(4));
  } else if (etatsGainable == EGOUTTAGE_FROID) {
    affiche("Egouttage       ", 0);
    lcd.setCursor(15, 0);
    lcd.print(char(5));
  } else if (etatsGainable == COMMANDE_CHAUFFAGE) {
    affiche("Chauffage oFF   ", 0);
    lcd.setCursor(13, 0);
    lcd.print("   ");
  } else if (etatsGainable == TEMPO_COMPRESSEUR_CHAUFFAGE || etatsGainable == TEMPO_DEGIVRAGE || etatsGainable == MODE_DEGIVRAGE) {
    affiche("Chauffage    ", 0);
    if (digitalRead(relaisComp)) {
      lcd.setCursor(13, 0);
      lcd.print(char(8));
      if (modifVitesseInt == true) {
        lcd.setCursor(14, 0);
        lcd.print(vitesseTravail);
      } else if (digitalRead(relaisVentInt)) {
        lcd.setCursor(14, 0);
        lcd.print("4");
      } else {
        lcd.setCursor(14, 0);
        lcd.print("1");
      }
      lcd.setCursor(15, 0);
      lcd.print(char(2));
    }
  } else if (etatsGainable == DEGIVRAGE_NATUREL) {
    affiche("Degivrage      ", 0);
    lcd.setCursor(15, 0);
    lcd.print(char(4));
  } else if (etatsGainable == EGOUTTAGE_NATUREL) {
    affiche("Egouttage      ", 0);
    lcd.setCursor(15, 0);
    lcd.print(char(5));
  } else if (etatsGainable == TEMPO_DEG_V4V || etatsGainable == TEMPO_DEG_COMPRESSEUR || etatsGainable == DEGIVRAGE_ELECTRIC) {
    affiche("Degivrage     ", 0);
    lcd.setCursor(14, 0);
    lcd.print(char(6));
    lcd.setCursor(15, 0);
    lcd.print(char(4));
  } else if (etatsGainable == EGOUTTAGE_CHAUFFAGE) {
    affiche("Egouttage    ", 0);
    lcd.setCursor(14, 0);
    lcd.print(char(6));
    lcd.setCursor(15, 0);
    lcd.print(char(5));
  } else if (etatsGainable == FIN_EGOUTTAGE_CHAUFFAGE) {
    affiche("Fin Egouttage ", 0);
    lcd.setCursor(14, 0);
    lcd.print(char(6));
    lcd.setCursor(15, 0);
    lcd.print(char(5));
  } else if (etatsGainable == COMMANDE_CANICULE) {
    affiche("Canicul oFF ", 0);
    lcd.setCursor(12, 0);
    lcd.print(char(7));
    lcd.setCursor(13, 0);
    lcd.print("   ");
  } else if (etatsGainable == TEMPO_V4V_CANICULE || etatsGainable == TEMPO_COMPRESSEUR_CANICULE || etatsGainable == COMPRESSEUR_CANICULE) {
    affiche("Canicul    ", 0);
    lcd.setCursor(12, 0);
    lcd.print(char(7));
    lcd.setCursor(13, 0);
    lcd.print(char(8));
    if (digitalRead(relaisVentInt)) {
      lcd.setCursor(14, 0);
      lcd.print("4");
    }
    if (digitalRead(relaisComp)) {
      lcd.setCursor(15, 0);
      lcd.print(char(1));
    }
  }
}

// les etats du gainable possible pour modifier les vitesses interieur
void vitesseGainable() { // les etats Gainable pour modifier vitesse ventilateur interieur
  if (etatsGainable == TEMPO_V4V || etatsGainable == TEMPO_COMPRESSEUR_FROID || etatsGainable == COMPRESSEUR_FROID || etatsGainable == TEMPO_DEGIVRAGE || etatsGainable == MODE_DEGIVRAGE) {
    etablirModeVitesse();
  } else {
    etablirModeEteint();
  }
}

// controle des temperatures pour l'automatisation
void controleTemperature() { // temporisation de controle temperature exterieur
  if (millis() - chronoAutoTemp >= AutoTemp) {
    etatsGainable = DEPART;
    chronoAutoTemp = millis();
  }
}

// le gainable
void gainable() {

  if (departGainable == false) {
    etatsGainable = ARRET;
  }

  switch (etatsGainable) {

    case ARRET:

      if (departGainable == true) {
        etatsGainable = DEPART;
      } else {
        desactiveTousRelais();
        digitalWrite(ledCa, LOW);
        digitalWrite(ledFr, LOW);
        digitalWrite(ledCh, LOW);
      }

      break;

    case DEPART:  // controle automatique de la temperature pour selectionner le mode

      if (nettoyageFiltre >= 4250000ul) {  // 49 jours de fonctionnement du compresseur 4250000000ms
        entretienFiltre = true;
        etatsGainable = FILTRE;
      } else if (tempExt) {  // si la temperature exterieur est inferieur as la consigne (12.5°C)
        etatsGainable = COMMANDE_CHAUFFAGE;              // j'active le CHAUFFAGE
      } else if (tempExtLue <= consignes[1].consigne) {  // si non si la temperature exterieur est inferieur a la consigne canicule (30°C)
        etatsGainable = COMMANDE_FROID;
      } else {  // si non
        etatsGainable = COMMANDE_CANICULE;  // j'active FROID CANICULE
      }
      break;

    case COMMANDE_FROID:

      if (fonctionFr == true) {
        tempoV4VFr = millis();
        etatsGainable = TEMPO_V4V;
      } else {
        digitalWrite(ledFr, LOW);
        desactiveTousRelais();
        controleTemperature();
      }
      break;

    case TEMPO_V4V:

      if (fonctionFr == false) {
        etatsGainable = COMMANDE_FROID;
        chronoAutoTemp = millis();
      } else if (millis() - tempoV4VFr >= 6000ul) {  // temporisation de 1 minute
        tempoCompFr = millis();
        digitalWrite(relaisV4V, HIGH);
        etatsGainable = TEMPO_COMPRESSEUR_FROID;
      } else {
        activeRelaisVentExtFroid();
        digitalWrite(ledFr, HIGH);
        digitalWrite(relaisVentUniteInt, HIGH);  // relais ventilateur unité interieur oN
        if (modifVitesseInt == false) {
          activeRelaisVentIntFroid();
        }  else {
          vitesseInterieur();
        }
      }
      break;

    case TEMPO_COMPRESSEUR_FROID:

      if (fonctionFr == false) {
        etatsGainable = COMMANDE_FROID;
        chronoAutoTemp = millis();
      } else if (millis() - tempoCompFr >= 18000ul) {  // temporisation de 3 minutes avant demarrage compresseur 180000ul
        departChronoFiltre = millis();
        etatsGainable = COMPRESSEUR_FROID;
      } else {
        activeRelaisVentExtFroid();
        if (modifVitesseInt == false) {
          activeRelaisVentIntFroid();
        }  else {
          vitesseInterieur();
        }
      }
      break;

    case COMPRESSEUR_FROID:

      if (fonctionFr == false) {
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        desactiveTousRelais();
        digitalWrite(ledFr, LOW);
        chronoAutoTemp = millis();
        etatsGainable = COMMANDE_FROID;
      } else if (tempEchangeurIntLue <= consignes[11].consigne) {  // si : temperature echangeur interieur est inferieur ou egal 1°C (lancement degivrage)
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        vitesseTravail = vitesses[0].vitesse;
        etatsGainable = DEGIVRAGE_FROID;
      } else {
        activeRelaisVentExtFroid();
        digitalWrite(relaisComp, HIGH);
        if (modifVitesseInt == false) {
          activeRelaisVentIntFroid();
        }  else {
          vitesseInterieur();
        }
      }
      break;

    case DEGIVRAGE_FROID:

      if (tempEchangeurIntLue >= consignes[12].consigne) {
        tempoEgouttageFr = millis();
        etatsGainable = EGOUTTAGE_FROID;
      } else {
        desactiveTousRelais();
        activeLedDegivrageFr();
        activeLedDegivrageCa();
      }
      break;

    case EGOUTTAGE_FROID:  // etat EGOUTTAGE_FR

      if (millis() - tempoEgouttageFr >= 120000ul) {  // 120000ms = 2 minutes
        desactiveTousRelais();
        caniculeLed = false;
        etatsGainable = DEPART;
      } else {
        activeLedDegivrageFr();
        activeLedDegivrageCa();
        activeRelaisVentIntDegFr();
      }
      break;

    case COMMANDE_CHAUFFAGE:

      if (fonctionCh == true) {
        tempoCompCh = millis();
        etatsGainable = TEMPO_COMPRESSEUR_CHAUFFAGE;
      } else {
        digitalWrite(ledCh, LOW);
        desactiveTousRelais();
        controleTemperature();
      }
      break;

    case TEMPO_COMPRESSEUR_CHAUFFAGE:

      if (fonctionCh == false) {
        chronoAutoTemp = millis();
        etatsGainable = COMMANDE_CHAUFFAGE;
      } else if (millis() - tempoCompCh >= 18000ul) {  // temporisation de 3 minutes avant le demarage du compresseur 180000ul
        departChronoFiltre = millis();
        tempoDegCh = millis();
        etatsGainable = TEMPO_DEGIVRAGE;
      } else {
        digitalWrite(ledCh, HIGH);
      }
      break;

    case TEMPO_DEGIVRAGE:

      if (fonctionCh == false) {
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        chronoAutoTemp = millis();
        etatsGainable = COMMANDE_CHAUFFAGE;
      } else if (millis() - tempoDegCh >= 2400000ul) {  // si : la temporisation de 40 minutes (si compresseur a fonctionner pendant 40 minutes) 2400000ul
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        etatsGainable = MODE_DEGIVRAGE;
      } else {
        digitalWrite(relaisComp, HIGH);
        activeRelaisVentExtCh();
        if (tempEchangeurIntLue >= consignes[10].consigne) {  //
          digitalWrite(relaisVentUniteInt, HIGH);             // demarrage ventilateur interieur a 30°C coupure au neutre
        }
        if (modifVitesseInt == false) {
          activeRelaisVentIntCh();
        } else {
          vitesseInterieur();
        }
      }
      break;

    case MODE_DEGIVRAGE:

      if (tempUnitExtLue >= consignes[5].consigne) {  // si : la temperature de l'unité exterieur est supperieur ou egal a 5°C ( lancement degivrage naturel )
        tempoDegNat = millis();
        etatsGainable = DEGIVRAGE_NATUREL;                        // passe a l'etat DEGIVRAGE_NATUREL
      } else if (tempEchangeurExtLue <= consignes[6].consigne) {  // si : la temperature de l'Echangeur exterieur est inferieur ou egal a -3°C ( degivrage gaz chaud inversion de cycle )
        tempoV4VCh = millis();
        etatsGainable = TEMPO_DEG_V4V;
      } else {
        activeRelaisVentExtCh();
        if (modifVitesseInt == false) {
          activeRelaisVentIntCh();
        } else {
          vitesseInterieur();
        }
      }
      break;

    case DEGIVRAGE_NATUREL:  // etat DEGIVRAGE_NATUREL

      if (millis() - tempoDegNat >= 600000ul) {  // 600000ms = 10 minutes
        tempoEgouttageNat = millis();
        etatsGainable = EGOUTTAGE_NATUREL;  // passe a l'etat EGOUTTAGE_NATUREL
      } else {
        desactiveTousRelais();
        activeLedDegivrageCh();
      }
      break;

    case EGOUTTAGE_NATUREL:  // etat EGOUTTAGE_NATUREL

      if (millis() - tempoEgouttageNat >= 300000ul) {  // 300000ms
        desactiveTousRelais();
        etatsGainable = DEPART;  // passe a l'etat
      } else {
        vitesseTravail = vitesses[0].vitesse;
        activeRelaisVentExtDegCh();
        activeLedDegivrageCh();
      }
      break;

    case TEMPO_DEG_V4V:  // etat DEGIVRAGE_ELECTRIC

      if (millis() - tempoV4VCh >= 78000ul) {  // 78000ms 1 minute 30 secondes
        tempoCompCh = millis();
        etatsGainable = TEMPO_DEG_COMPRESSEUR;
      } else {
        desactiveTousRelais();
        activeLedDegivrageCh();
      }
      break;

    case TEMPO_DEG_COMPRESSEUR:

      if (millis() - tempoCompCh >= 120000ul) {  // 120000ms 2 minutes
        etatsGainable = DEGIVRAGE_ELECTRIC;
      } else {
        digitalWrite(relaisV4V, HIGH);  // relais vanne 4 voies oN
        activeLedDegivrageCh();
      }
      break;

    case DEGIVRAGE_ELECTRIC:

      if (tempEchangeurExtLue >= consignes[7].consigne) {  // si la temperature est superieur ou egal a 12.5°C ( fin de degivrage )
        tempoEgouttageEle = millis();
        etatsGainable = EGOUTTAGE_CHAUFFAGE;  // passe a l'etat EGOUTTAGE_ELECTRIC
      } else {
        digitalWrite(relaisComp, HIGH);  // relais compresseur oN
        activeLedDegivrageCh();
      }
      break;

    case EGOUTTAGE_CHAUFFAGE:  // etat EGOUTTAGE_ELECTRIC

      if (millis() - tempoEgouttageEle >= 300000ul) {  // 300000ms = 5 minutes
        tempoFinEgouttageEle = millis();
        etatsGainable = FIN_EGOUTTAGE_CHAUFFAGE;  // passe a l'etat FIN_EGOUTTAGE_ELE
      } else {
        desactiveTousRelais();
        activeLedDegivrageCh();
      }
      break;

    case FIN_EGOUTTAGE_CHAUFFAGE:  // etat FIN_EGOUTTAGE_ELE

      if (millis() - tempoFinEgouttageEle >= 150000ul) {  // 150000ms = 2.5 minutes
        desactiveTousRelais();
        etatsGainable = DEPART;  // passe a l'etat
      } else {
        vitesseTravail = vitesses[0].vitesse;
        activeRelaisVentExtDegCh();
        activeLedDegivrageCh();
      }
      break;

    case COMMANDE_CANICULE:  // etat FROID_CANICULE

      if (fonctionCa == true) {
        tempoV4VFr = millis();
        etatsGainable = TEMPO_V4V_CANICULE;
      } else {
        digitalWrite(ledCa, LOW);
        controleTemperature();
        desactiveTousRelais();
      }
      break;

    case TEMPO_V4V_CANICULE:

      if (fonctionCa == false) {
        etatsGainable = COMMANDE_CANICULE;
        chronoAutoTemp = millis();
      } else if (millis() - tempoV4VFr >= 6000ul) {  // temporisation de 1 minute
        tempoCompFr = millis();
        digitalWrite(relaisV4V, HIGH);
        etatsGainable = TEMPO_COMPRESSEUR_CANICULE;
      } else {
        activeRelaisVentsCanicule();
        digitalWrite(ledCa, HIGH);
        digitalWrite(relaisVentUniteInt, HIGH);  // relais ventilateur unité interieur oN
      }
      break;

    case TEMPO_COMPRESSEUR_CANICULE:

      if (fonctionCa == false) {
        chronoAutoTemp = millis();
        etatsGainable = COMMANDE_CANICULE;
      } else if (millis() - tempoCompFr >= 18000ul) {  // temporisation de 3 minutes avant demarrage compresseur
        departChronoFiltre = millis();
        etatsGainable = COMPRESSEUR_CANICULE;
      } else {
        activeRelaisVentsCanicule();
      }
      break;

    case COMPRESSEUR_CANICULE:

      if (fonctionCa == false) {
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        digitalWrite(ledCa, LOW);
        chronoAutoTemp = millis();
        etatsGainable = COMMANDE_CANICULE;
      } else if (tempEchangeurIntLue <= consignes[11].consigne) {  // si : temperature echangeur interieur est inferieur ou egal 1°C (lancement degivrage)
        finChronoFiltre = millis();
        nettoyageFiltre = (finChronoFiltre - departChronoFiltre) + nettoyageFiltre;
        caniculeLed = true;
        etatsGainable = DEGIVRAGE_FROID;
      } else {
        digitalWrite(relaisComp, HIGH);
      }
      break;

    case FILTRE:

      if (millis() - tempoLedArretCompletProgramClignote >= 500) {
        tempoLedArretCompletProgramClignote = millis();
        ledChFrCaCl = (ledChFrCaCl == HIGH) ? LOW : HIGH;
        digitalWrite(ledFr, ledChFrCaCl);
        digitalWrite(ledCa, ledChFrCaCl);
        digitalWrite(ledCh, ledChFrCaCl);
        if (capteurFiltre.pressedFor(ms)) {
          entretienFiltre = false;
          nettoyageFiltre = 0;
          etablirModeEteint();
        }
      }
      break;
  }
}

// la fonction calculs
void autresCalculs() {
  hysteresisTempIntCa();
  hysteresisTempExt();
  hysteresisTempVentIntCh();
  hysteresisTempVentExtCh();
  hysteresisTempVentIntFr();
  hysteresisTempVentExtFr();
  commandeFroid();
  commandeChauffage();
  commandeCanicule();
  consigneIntCa = tempExtLue - consignes[13].consigne;  // consigne interieur canicule est egale a la temperature exterieur (NORD) - temperature delta (-6)
}

// l'affichage Serial
void imprimeTout() {
  const unsigned long deltaT = 2500ul;    // en ms, toutes les 2,5 secondes
  static unsigned long chrono = -deltaT;  // valeur initiale pour affichage au premier appel

  if (millis() - chrono >= deltaT) {
    Serial.print(F("Temperature EXT °C = "));
    Serial.print(tempExtLue);
    Serial.print(F(" ; Temperature Unite EXT °C = "));
    Serial.print(tempUnitExtLue);
    Serial.print(F(" ; Temperature Echangeur EXT °C = "));
    Serial.println(tempEchangeurExtLue);
    Serial.print(F(" ; Temperature Unite INT °C = "));
    Serial.print(tempUnitIntLue);
    Serial.print(F(" ; Temperature Echangeur INT °C = "));
    Serial.println(tempEchangeurIntLue);

    const char* nomDesEtats[] = { "ARRET", "DEPART", "COMMANDE_FROID", "TEMPO_V4V", "TEMPO_COMPRESSEUR_FROID", "COMPRESSEUR_FROID", "DEGIVRAGE_FROID", "EGOUTTAGE_FROID", "COMMANDE_CHAUFFAGE", "TEMPO_COMPRESSEUR_CHAUFFAGE", "TEMPO_DEGIVRAGE", "MODE_DEGIVRAGE", "DEGIVRAGE_NATUREL", "EGOUTTAGE_NATUREL", "TEMPO_DEG_V4V", "TEMPO_DEG_COMPRESSEUR", "DEGIVRAGE_ELECTRIC", "EGOUTTAGE_CHAUFFAGE", "FIN_EGOUTTAGE_CHAUFFAGE", "COMMANDE_CANICULE", "TEMPO_V4V_CANICULE", "TEMPO_COMPRESSEUR_CANICULE", "COMPRESSEUR_CANICULE", "FILTRE" };
    Serial.print(F("ETAT: "));
    Serial.println(nomDesEtats[etatsGainable]);

    const char* nomDesEtatsMode[] = { "ETEINT", "CONSIGNE", "DETAIL_CONSIGNE", "HYSTERESIS", "DETAIL_HYSTERESIS", "VITESSE", "ACTIF", "AUTOMATIQUE", "ERREUR", "NETTOYAGE_FILTRE" };
    Serial.print(F("MODE: "));
    Serial.println(nomDesEtatsMode[mode]);

    Serial.print(F("CHRONO = "));
    Serial.println(millis());

    Serial.print(F("Nettoyage filtre = "));
    Serial.println(nettoyageFiltre);

    Serial.print(F("Consigne Interieur CANICULE °C = "));
    Serial.println(consigneIntCa);

    Serial.print(F("tempo Controle Temp = "));
    Serial.println(chronoAutoTemp);

    Serial.print(F("modifVitesse = "));
    Serial.println(modifVitesseInt);

    Serial.print(F("vitesseTravail = "));
    Serial.println(vitesseTravail);

    Serial.print(F("consigne vent Int = "));
    Serial.println(consignes[8].consigne);

    chrono = millis();
  }
}

// le setup
void setup() {

  pinMode(relaisComp, OUTPUT);            // Relais Compresseur
  pinMode(relaisV4V, OUTPUT);             // Relais Vanne 4 Voies
  pinMode(relaisVentUniteExt, OUTPUT);    // Relais Ventilateur Unité Exterieur oN/oFF ( coupur au neutre )
  pinMode(relaisVentExt, OUTPUT);         // Relais Ventilateur Exterieur grande et petite vitesse
  pinMode(relaisVentUniteInt, OUTPUT);    // Relais Ventilateur unité interieur oN/oFF  ( coupure au neutre )
  pinMode(relaisVentInt, OUTPUT);         // Relais Ventilateur Interieur grande et petite vitesse
  pinMode(relaisVentIntPv, OUTPUT);       // Relais ventilateur Interieur petite vitesse
  pinMode(relaisVentIntGv, OUTPUT);       // Relais ventilateur Interieur Grande Vitesse
  pinMode(ledCa, OUTPUT);                 // LED fonction HP
  pinMode(ledFr, OUTPUT);                 // LED fonction Froid
  pinMode(ledCh, OUTPUT);                 // LED fonction Chauffage
  pinMode(ledBoutonMenu, OUTPUT);         // LED Bouton Menu
  pinMode(ledBoutonValid, OUTPUT);        // LED Bouton Valid
  pinMode(ledBoutonPlus, OUTPUT);         // LED Bouton Plus
  pinMode(ledBoutonMoins, OUTPUT);        // LED Bouton Moins
  thermCh1.begin(thermCh1Pin);            // Contact NC/NO Thermostat Radio Chambre 1
  thermCh2.begin(thermCh2Pin);            // Contact NC/NO Thermostat Radio Chambre 2
  thermCh3.begin(thermCh3Pin);            // Contact NC/NO Thermostat Radio Chambre 3
  thermCh4.begin(thermCh4Pin);            // Contact NC/NO Thermostat Radio Chambre 4
  thermSalon.begin(thermSalonPin);        // Contact NC/NO Thermostat Radio Salon
  capteurFiltre.begin(capteurFiltrePin);  // Capteur Presence Filtre
  boutonMenu.begin(menuPin);              // Bouton Menu
  boutonValid.begin(validPin);            // Bouton Valid
  boutonPlus.begin(boutonPlusPin);        // Bouton Plus
  boutonMoins.begin(boutonMoinsPin);      // Bouton Moins

  for (Sonde& s : sondes) s.begin();

  for (size_t indice = 0; indice < nombreDeConsignes; indice++)
    ;

  Serial.begin(115200);

  int result = lcd.begin(nbColonnes, nbLignes);
  if (result) {
    Serial.print("LCD initialization failed: ");
    Serial.println(result);
    hd44780::fatalError(result);
  }

  lcd.createChar(1, flecheFr);
  lcd.createChar(2, flecheCh);
  lcd.createChar(4, degivrage);
  lcd.createChar(5, goutte);
  lcd.createChar(6, eclaire);
  lcd.createChar(7, soleil);
  lcd.createChar(8, ventilateur);

  etablirModeEteint();
}

// la loop
void loop() {

  gestionEtat();
  autresCalculs();
  imprimeTout();
}


int main(void)
{
  setup();
  while(1)
    loop();


/* tester les GPIOs
 cout << "set Pin26: " << pinMode(26,OUTPUT) << endl;
 digitalWrite(26,1);
 cout << "read 26 :out =1 => " << digitalRead(26) << endl;
 digitalWrite(26,0);
 cout << "read 26 :out =0 => " << digitalRead(26) << endl;
 cout << "set input 26 pull down:" << pinMode(26,INPUT_PULLDOWN) <<endl;
 cout << "read 26 in " << digitalRead(26) << endl;
 cout << "set input 26:" << pinMode(26,INPUT) <<endl;
 cout << "read 26 in " << digitalRead(26) << endl;
 cout << "set input pullup 26:" << pinMode(26,INPUT_PULLUP) <<endl;
 usleep(100000);
 cout << "read 26 in " << digitalRead(26) << endl;
*/
 release_gpiod();
  return 0;
}

