#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"

namespace clawd_detail {

inline constexpr int T = 24;
// Durées des gestes en millisecondes.
inline constexpr uint32_t TOUCHER_COURT = 600, APPUI_LONG = 700;
inline constexpr uint32_t DUREE_BALAYAGE = 1200, DELAI_MENU = 8000;
inline constexpr int BORD_BALAYAGE = 100, COURSE_BALAYAGE = 45, DERIVE_TOUCHER = 30;
inline constexpr uint32_t DUREE_SURSAUT = 300, DUREE_ETOURDI = 1500;
inline constexpr uint32_t DUREE_ETIREMENT = 700, PAS_SAUT = 110;
inline constexpr uint32_t DUREE_REVEIL = 2500, PAS_REVEIL = 125, DUREE_RETOUR_REVEIL = 375;
inline constexpr uint32_t DUREE_ZONE_DENTS = 30000, DUREE_BROSSAGE = 4 * DUREE_ZONE_DENTS;
inline constexpr uint32_t FREQUENCE_BROSSE = 6, DUREE_CHANGEMENT_DENTS = 2500, DUREE_BRAVO = 2000;
inline constexpr uint32_t ATTITUDE_MIN = 20000, ATTITUDE_MAX = 40000;
// Une image de repos au plus toutes les 84 ms, et une image BAS toutes les 125 ms.
inline constexpr uint32_t PAS_PERSO = 84, PAS_BAS = 125;
inline constexpr uint32_t PAS_PLUIE = 125;  // Huit pas de pluie par seconde au plus.
// Le profil SOIR se prolonge après minuit jusqu'à 7 h.
inline constexpr int HEURE_DEBUT_JOUR = 7;
// Luminosité du profil SOIR selon l'état de la lampe du bureau.
inline constexpr float SOIR_LAMPE_ETEINTE = 0.40f, SOIR_LAMPE_ALLUMEE = 0.85f;
inline constexpr uint32_t DELAI_LUMINOSITE = 10000;  // 10 s entre réglages automatiques.

struct PalettePerso {
  esphome::Color CORPS, CLAIR, OMBRE, JOUE, OEIL;
  esphome::Color LANGUE, LUN, VERRE, BLANC, REFLET;
};

struct Palette {
  esphome::Color BG, SOL, ONDE;
  esphome::Color TEXTE, DISCRET, PISTE, VALIDE, ECOUTE, REFLEXION, REPONSE, COEUR;
  esphome::Color NOEL, AVERTISSEMENT;
  esphome::Color DRAP, TXTF;
};

inline constexpr Palette C{
    {0x00, 0x00, 0x00}, {0x37, 0x20, 0x1b}, {0xcd, 0xbd, 0xad},
    {0xff, 0xe8, 0xd5}, {0x9a, 0x9e, 0xb2}, {0x33, 0x36, 0x44},
    {0x68, 0x85, 0x80}, {0x9d, 0xeb, 0xcd}, {0xff, 0xcd, 0x79}, {0xb5, 0xcd, 0xff},
    {0xff, 0xab, 0xbc}, {0xe6, 0x5a, 0x63}, {0xef, 0xc3, 0x53},
    {0x27, 0x21, 0x25}, {0xf0, 0x9a, 0x55}
};

inline constexpr PalettePerso C_PERSO{
    {0xe7, 0x83, 0x4b}, {0xf2, 0x96, 0x5d}, {0xc9, 0x68, 0x3c}, {0xd6, 0x69, 0x49}, {0x51, 0x2c, 0x26},
    {0xe4, 0x83, 0x6c}, {0x24, 0x1e, 0x22}, {0x39, 0x30, 0x35}, {0xff, 0xff, 0xff}, {0xb8, 0xa9, 0xad}
};
enum class Etat { INCONNU, CLAWD, ECOUTE, REFLEXION, REPONSE };
enum class Phase { JOUR, SOIR };
enum Zone : uint16_t {
  HEURE = 1, PERSO = 2, BAS = 4, DEUX_POINTS = 8, ICONE_ALERTE = 16, DATE = 32,
  MINUTEUR = 64, MENU_MINUTEUR = 128, MENU_BUREAU = 256, MENU_DEGUISEMENT = 512,
  COMPLET = 1024, PASTILLES_VOCALES = 2048,
  BROSSAGE_COMPLET = 4096, BROSSAGE_TEMPS = 8192, BROSSAGE_PERSO = 16384, BROSSAGE_CONSIGNE = 32768
};
enum class Reaction : uint8_t { AUCUNE, SURSAUT, CALIN, ETOURDI, CONTENT, TRISTE, SAUT, REVEIL, RETOUR_REVEIL };
enum class Issue { AUCUNE, EN_COURS, REUSSITE, ERREUR };
// Le select utilise les indices 0 à 2. Matrix garde la valeur 6 mémorisée depuis la mission 7.
enum class Costume : uint8_t { AUCUN = 1, NOEL = 2, MATRIX = 6 };
enum class Icone : uint8_t { STEF, ATTENTION, INFO };
enum Yeux { OUVERTS, FERMES, ECARQUILLES, ARCS, BAISSES, CROIX, PLISSES };
enum class Attitude : uint8_t { AUCUNE, REGARDER, REBONDIR, BAILLER, CAFE, DRAPEAU };
enum class Transition : uint8_t { CLIGNER, ETIRER, SAUTER };
enum class Climat : uint8_t { NORMAL, CHAUD, FROID };

inline Costume costume_actif(size_t option, int mois, int jour, int etape_matrix) {
  if (etape_matrix == int(Costume::MATRIX)) return Costume::MATRIX;
  if (option == size_t(Costume::AUCUN)) return Costume::AUCUN;
  if (option == size_t(Costume::NOEL)) return Costume::NOEL;
  return mois == 12 && jour >= 1 && jour <= 25 ? Costume::NOEL : Costume::AUCUN;
}

inline Costume costume_suivant(Costume costume) {
  if (costume == Costume::AUCUN) return Costume::NOEL;
  if (costume == Costume::NOEL) return Costume::MATRIX;
  return Costume::AUCUN;
}

struct Alerte {
  std::string lignes[2];
  Icone icone = Icone::INFO;
  int bloc = 1;
  uint32_t duree_ms = 0;
  uint32_t revision = 0;
  uint32_t debut = 0;
  bool active = false;

  void effacer() {
    active = false;
    duree_ms = 0;
    for (auto &ligne : lignes) ligne.clear();
  }

  void activer(uint32_t t) {
    active = true;
    debut = t;
  }

  void suspendre(uint32_t t) {
    duree_ms -= std::min(duree_ms, t - debut);
    active = false;
  }
};

inline Alerte alerte;

// dy, écrasement, bras gauche, bras droit.
inline constexpr int8_t JUMP[][4] = {
    {0, 1, 1, 1}, {-1, 0, 0, 0}, {-2, 0, -1, -1},
    {-3, 0, -2, -1}, {-3, 0, -2, -1}, {-2, 0, -1, 0},
    {-1, 0, 0, 0}, {0, 1, 1, 1}, {0, 0, 0, 0}
};
inline constexpr uint32_t DUREE_SAUT = sizeof(JUMP) / sizeof(JUMP[0]) * PAS_SAUT;
// Durées des transitions, dans l'ordre de l'enum.
inline constexpr uint32_t DUREES_TRANSITION[] = {450, DUREE_ETIREMENT, DUREE_SAUT};

struct Frame {
  // lun relève les lunettes.
  // effet et pas déterminent les cœurs, étoiles et étincelles.
  // bouche : 0 sourire, 1 ouverte, 2 détendue, 3 triste.
  int8_t dx, dy, ecrase, gauche, droit, lun, yeux, regard, bouche;
  Reaction effet;
  int8_t pas;
  int8_t dx_px, dy_px;  // Frisson et halètement, en pixels.
  int8_t fanion;  // Ondulation du bord en quarts de case.
  uint8_t vapeur;  // Zéro sans tasse, 1 à 4 pour la hauteur de la vapeur.
  Costume costume = Costume::AUCUN;
  bool ondes, etirement, drapeau;
};
static_assert(std::has_unique_object_representations_v<Frame>);

struct Repos {
  Attitude attitude = Attitude::AUCUNE;
  Transition transition = Transition::CLIGNER;
  Climat climat = Climat::NORMAL;
  uint32_t graine = 0, temps = 0, dernier_tick = 0;
  uint32_t debut = 0, duree = 0, debut_etirement = 0;
  bool actif = false;
  bool lampe_connue = false, lampe_allumee = false;
  bool etirement_demande = false, etirement_lampe = false;

  uint32_t tirer() {
    // Décalages de xorshift32. La graine vient du premier tick de repos.
    graine ^= graine << 13;
    graine ^= graine >> 17;
    graine ^= graine << 5;
    return graine;
  }

  void changer_attitude(bool reactions) {
    constexpr Attitude choix[] = {
        Attitude::REGARDER, Attitude::REBONDIR, Attitude::BAILLER, Attitude::CAFE, Attitude::DRAPEAU
    };
    const auto poids = [&](Attitude candidate) {
      if (candidate == attitude) return 0;
      // Lampe allumée : le rebond a deux fois plus de chances que chaque autre attitude.
      return reactions && lampe_allumee && candidate == Attitude::REBONDIR ? 2 : 1;
    };
    int total = 0;
    for (const auto candidate : choix) total += poids(candidate);
    int tirage = tirer() % total;
    for (const auto candidate : choix) {
      tirage -= poids(candidate);
      if (tirage < 0) {
        attitude = candidate;
        break;
      }
    }
    transition = static_cast<Transition>(tirer() % (sizeof(DUREES_TRANSITION) / sizeof(DUREES_TRANSITION[0])));
    debut = temps;
    duree = ATTITUDE_MIN + tirer() % (ATTITUDE_MAX - ATTITUDE_MIN + 1);
  }

  void changer_lampe(bool allumee, bool reactions) {
    // Le premier état reçu de Home Assistant ne constitue pas une extinction.
    if (reactions && lampe_connue && lampe_allumee && !allumee) etirement_demande = true;
    lampe_connue = true;
    lampe_allumee = allumee;
  }

  void couper_reactions() {
    climat = Climat::NORMAL;
    etirement_demande = etirement_lampe = false;
  }
};

struct Animation {
  Frame courante{};
  Frame dessinee{};
  Repos repos;
  uint32_t revision_alerte = 0, debut_signe = 0;
};

inline Animation animation;

struct Interaction {
  Reaction reaction = Reaction::AUCUNE;
  uint32_t debut_reaction = 0;
  Frame pose_retour_reveil{};
  bool retour_reveil_commence = false;
  bool appui = false;
  bool appui_perso = false;
  bool calin_joue = false;
  uint32_t debut_appui = 0;
  uint8_t doigt = 0;
  int x_depart = 0, x_courant = 0, y_depart = 0;
  int deplacement_max_carre = 0;
  bool balayage = false;
  int regard = 0;

  void jouer(Reaction nouvelle, uint32_t t) {
    reaction = nouvelle;
    debut_reaction = t;
  }

  bool a_derive() const { return deplacement_max_carre > DERIVE_TOUCHER * DERIVE_TOUCHER; }

  void suivre(int x, int y) {
    x_courant = x;
    const int dx = x - x_depart, dy = y - y_depart;
    deplacement_max_carre = std::max(deplacement_max_carre, dx * dx + dy * dy);
    // Zone neutre de deux cases autour du milieu du personnage.
    regard = x < 9 * T ? -1 : x >= 11 * T ? 1 : 0;
  }
};

inline Interaction interaction;

inline void terminer_reveil() {
  auto &g = interaction;
  if (g.reaction != Reaction::REVEIL) return;
  // Repartir de la pose réellement affichée, y compris après un arrêt du son.
  g.pose_retour_reveil = animation.dessinee;
  g.retour_reveil_commence = false;
  g.reaction = g.pose_retour_reveil.effet == Reaction::REVEIL ? Reaction::RETOUR_REVEIL : Reaction::AUCUNE;
}

struct Menu {
  bool ouvert = false;
  bool doigt_pose = false;
  uint32_t dernier_toucher = 0;
  uint32_t ferme_a = 0u - 5000;

  void toucher(uint32_t t) {
    doigt_pose = true;
    dernier_toucher = t;
  }

  void relacher(uint32_t t) {
    if (!doigt_pose) return;
    doigt_pose = false;
    dernier_toucher = t;
  }

  bool inactif(uint32_t t) const { return ouvert && !doigt_pose && t - dernier_toucher >= DELAI_MENU; }
};

inline Menu menu;

struct Minuteur {
  enum class Mode { ORDINAIRE, BROSSAGE, CARILLON_BROSSAGE };

  uint32_t debut = 0, duree = 0;
  Mode mode = Mode::ORDINAIRE;
  bool ecoule = false;
  uint8_t groupes_joues = 0;
  bool sortie_sonore = false;
  size_t octets_envoyes = 0;
  int choix = 5;  // Durée choisie dans le menu, en minutes (1 à 99).

  void lancer(uint32_t t, int minutes) {
    lancer_pour(t, minutes * 60000u, Mode::ORDINAIRE);
  }

  void lancer_pour(uint32_t t, uint32_t duree_ms, Mode nouveau_mode) {
    debut = t;
    duree = duree_ms;
    mode = nouveau_mode;
    ecoule = false;
    groupes_joues = 0;
  }

  void actualiser(uint32_t t) {
    if (duree && t - debut >= duree) ecoule = true;
  }

  void effacer() {
    duree = 0;
    ecoule = false;
    mode = Mode::ORDINAIRE;
  }

  int affichage(uint32_t t) const {
    if (!duree || mode == Mode::CARILLON_BROSSAGE) return -1;
    if (ecoule) return 0;
    return (duree - (t - debut) + 999) / 1000;
  }
};

inline Minuteur minuteur;

inline const auto &pcm_melodie_minuteur() {
  constexpr int frequence_pcm = 16000;
  constexpr int pas = frequence_pcm / 5;  // Une note toutes les 200 ms.
  constexpr int duree_note = 2 * pas;
  constexpr int echantillons = 6 * pas;
  static const auto pcm = [] {
    constexpr float notes[] = {1046.50f, 1318.51f, 1567.98f, 1318.51f, 1046.50f};  // Do, mi, sol, mi, do (octave haut : le petit haut-parleur rend mal les graves).
    constexpr float deux_pi = 6.283185307f;
    std::array<uint8_t, echantillons * 2> data{};
    static_assert(echantillons == (std::size(notes) - 1) * pas + duree_note);
    for (size_t n = 0; n < std::size(notes); ++n) {
      const float rotation = deux_pi * notes[n] / frequence_pcm;
      for (int i = 0; i < duree_note; ++i) {
        const float phase = rotation * i;
        const float attaque = std::min(i / 128.0f, 1.0f);
        const float queue = 1.0f - float(i) / (duree_note - 1);
        const float onde = 0.82f * std::sin(phase) + 0.18f * std::sin(phase * 2.01f);
        // Deux notes qui se chevauchent restent sous 18000 (limite int16 : 32767).
        const int16_t note = 9000 * attaque * queue * queue * onde;
        const size_t j = 2 * (n * pas + i);
        const uint16_t valeur = (data[j] | (data[j + 1] << 8)) + note;
        data[j] = static_cast<uint8_t>(valeur);
        data[j + 1] = static_cast<uint8_t>(valeur >> 8);
      }
    }
    return data;
  }();
  return pcm;
}

inline Etat lire_etat(const std::string &etat) {
  if (etat == "J'écoute...") return Etat::ECOUTE;
  if (etat == "Je réfléchis...") return Etat::REFLEXION;
  if (etat == "Je réponds...") return Etat::REPONSE;
  return Etat::CLAWD;
}

inline void pose_saut(Frame &f, uint32_t e) {
  const auto &pas = JUMP[e / PAS_SAUT];
  f.dy = pas[0];
  f.ecrase = pas[1];
  f.gauche = pas[2];
  f.droit = pas[3];
}

inline void pose_reveil(Frame &f, uint32_t e) {
  f.effet = Reaction::REVEIL;
  f.lun = 2;
  f.yeux = ECARQUILLES;
  f.bouche = e < 3 * PAS_REVEIL ? 1 : 0;
  f.gauche = f.droit = -2;
  if (e < 2 * PAS_REVEIL) {
    f.dy = -1;
    f.dy_px = e >= PAS_REVEIL ? T / 2 : 0;
  } else if (e >= DUREE_REVEIL - 2 * PAS_REVEIL) {
    // Pose centrée que l'écran conserve pendant le carillon.
    f.gauche = f.droit = -1;
  } else if (e >= 3 * PAS_REVEIL) {
    f.dx = (e / PAS_REVEIL) % 2 ? -1 : 1;
  }
}

inline void pose_danse(Frame &f, uint32_t e, int dir) {
  const int s = e / 200 % 4;  // Quatre pas de danse.
  const int cote = s < 2 ? dir : -dir;
  f.dx = s % 2 ? 0 : cote;
  f.dy = s % 2 ? -1 : 0;
  f.gauche = cote;
  f.droit = -cote;
}

inline Climat mesurer_climat(Climat precedent, float temperature) {
  constexpr float CHAUD = 26.0f, FROID = 17.0f, MARGE = 0.5f;  // Seuils et retour en °C.
  if (!std::isfinite(temperature)) return Climat::NORMAL;
  if (temperature > (precedent == Climat::CHAUD ? CHAUD - MARGE : CHAUD)) return Climat::CHAUD;
  if (temperature < (precedent == Climat::FROID ? FROID + MARGE : FROID)) return Climat::FROID;
  return Climat::NORMAL;
}

inline void pose_etirement(Frame &f) {
  f.gauche = f.droit = -3;  // Bras levés de trois cases, puis prolongés sur les côtés.
  f.etirement = true;
}

inline void pose_transition(Frame &f, Transition transition, uint32_t e) {
  switch (transition) {
    case Transition::CLIGNER:
      f.lun = 2;
      f.yeux = e < 220 ? FERMES : OUVERTS;  // Réouverture des yeux avant l'activité.
      break;
    case Transition::ETIRER:
      pose_etirement(f);
      f.lun = 2;
      f.yeux = ARCS;
      break;
    case Transition::SAUTER:
      pose_saut(f, e);
      break;
  }
}

inline void calcul_repos(Repos &r, Frame &f, uint32_t t, bool reactions) {
  if (r.etirement_demande) {
    r.etirement_demande = false;
    r.etirement_lampe = true;
    r.debut_etirement = t;
  }
  if (r.etirement_lampe && t - r.debut_etirement >= DUREE_ETIREMENT) r.etirement_lampe = false;
  if (r.climat != Climat::NORMAL || r.etirement_lampe) {
    r.actif = false;
    f.lun = 2;
    if (r.climat == Climat::CHAUD) {
      f.yeux = PLISSES;
      f.bouche = 1;
      f.dy_px = -int(t / 250 % 2);  // Soulève le corps d'un pixel toutes les 250 ms.
    } else if (r.climat == Climat::FROID) {
      f.yeux = ECARQUILLES;
      f.bouche = 2;
      f.dx_px = t / 125 % 2;  // Alterne d'un pixel toutes les 125 ms.
    }
    if (r.etirement_lampe) pose_etirement(f);
    return;
  }

  // Le compteur avance uniquement quand Clawd peut jouer son attitude de repos.
  if (r.actif) r.temps += t - r.dernier_tick;
  r.dernier_tick = t;
  r.actif = true;
  if (r.attitude == Attitude::AUCUNE) {
    r.graine = t ? t : 1;  // xorshift32 exige une graine non nulle.
    r.changer_attitude(reactions);
  } else if (r.temps - r.debut >= r.duree) {
    r.changer_attitude(reactions);
  }

  const uint32_t e = r.temps - r.debut;
  // La transition se joue à la fin du créneau de 20 à 40 secondes.
  const uint32_t debut_transition = r.duree - DUREES_TRANSITION[uint8_t(r.transition)];
  if (e >= debut_transition) {
    pose_transition(f, r.transition, e - debut_transition);
    return;
  }
  switch (r.attitude) {
    case Attitude::REGARDER: {
      // Regarde de chaque côté 600 ms toutes les quatre secondes.
      const uint32_t regard = e % 4000;
      if (regard < 1200) {
        f.lun = 2;
        f.regard = regard < 600 ? -1 : 1;
      }
      break;
    }
    case Attitude::REBONDIR: {
      // Une case en hauteur, puis amorti à l'atterrissage, toutes les cinq secondes.
      const uint32_t rebond = e % 5000;
      if (rebond < 250) {
        f.dy = -1;
        f.gauche = f.droit = -1;
      } else if (rebond < 400) {
        f.ecrase = f.gauche = f.droit = 1;
      }
      break;
    }
    case Attitude::BAILLER:
      if (e % 8000 < 900) {  // Bâillement bref, puis retour au sourire.
        f.lun = 2;
        f.yeux = PLISSES;
        f.bouche = 1;
        f.gauche = -1;
      }
      break;
    case Attitude::CAFE:
      f.vapeur = 1 + e / 250 % 4;  // Quatre hauteurs de vapeur par seconde.
      break;
    case Attitude::DRAPEAU: {
      constexpr int8_t ondulation[] = {0, -1, 0, 1};
      const int pas = e / 430 % 4;  // Un geste de la pince en quatre temps.
      f.drapeau = true;
      f.droit = pas == 1 || pas == 2 ? -1 : 0;
      f.fanion = ondulation[pas];
      break;
    }
    case Attitude::AUCUNE:
      break;
  }
}

inline void calcul_reaction(Frame &f, uint32_t t, bool reactions) {
  auto &g = interaction;
  const uint32_t e = t - g.debut_reaction;
  const auto r = g.reaction;
  constexpr uint32_t DUREE_ANIMEE = 1500, DUREE_TRISTE = 2000;
  switch (r) {
    case Reaction::SURSAUT:
      if (e < DUREE_SURSAUT) {
        f.lun = 2;
        f.yeux = ECARQUILLES;
        f.bouche = 1;
        f.dy = -1;
        return;
      }
      break;
    case Reaction::CALIN:
      if (e < DUREE_ANIMEE) {
        pose_danse(f, e, 1);
        f.lun = 1;
        f.effet = r;
        f.pas = e / 125;  // Montée des cœurs en douze pas.
        return;
      }
      break;
    case Reaction::ETOURDI:
      if (reactions && e < DUREE_ETOURDI) {
        f.lun = 2;
        f.yeux = CROIX;
        f.bouche = 2;
        f.effet = r;
        f.pas = e / 125;  // Rotation des étoiles.
        return;
      }
      break;
    case Reaction::CONTENT:
      if (e < DUREE_ANIMEE) {
        f.lun = 1;
        f.yeux = ARCS;
        f.effet = r;
        f.pas = e / 250;  // Étincelles alternées.
        return;
      }
      break;
    case Reaction::TRISTE:
      if (e < DUREE_TRISTE) {
        f.lun = 1;
        f.yeux = BAISSES;
        f.bouche = 3;
        f.effet = r;
        f.pas = e / 200;  // Chute de la larme.
        return;
      }
      break;
    case Reaction::SAUT:
      if (reactions && e < DUREE_SAUT) {
        pose_saut(f, e);
        return;
      }
      break;
    case Reaction::REVEIL:
      pose_reveil(f, e);
      return;
    case Reaction::RETOUR_REVEIL: {
      if (!g.retour_reveil_commence) {
        g.retour_reveil_commence = true;
        g.debut_reaction = t;
      }
      const int ecoule = std::min(t - g.debut_reaction, DUREE_RETOUR_REVEIL);
      constexpr int duree = DUREE_RETOUR_REVEIL;
      if (ecoule == duree) break;
      const auto costume = f.costume;
      f = g.pose_retour_reveil;
      f.costume = costume;
      const int reste = duree - ecoule;
      f.dx_px -= (f.dx * T + f.dx_px) * ecoule / duree;
      f.dy_px -= (f.dy * T + f.dy_px) * ecoule / duree;
      f.gauche = f.gauche * reste / duree;
      f.droit = f.droit * reste / duree;
      if (ecoule >= int(PAS_REVEIL)) {
        f.effet = Reaction::AUCUNE;
        f.lun = f.bouche = 0;
        f.yeux = OUVERTS;
      }
      return;
    }
    case Reaction::AUCUNE:
      break;
  }
  g.reaction = Reaction::AUCUNE;
}

inline bool pose_alerte(Animation &a, Frame &f, uint32_t t) {
  if (!alerte.active) return false;
  if (a.revision_alerte != alerte.revision) {
    a.revision_alerte = alerte.revision;
    a.debut_signe = t;
  }
  if (alerte.icone == Icone::STEF) {
    f.lun = 1;
    f.yeux = ARCS;
    // Une montée du bras, puis le salut reste en place.
    f.droit = t - a.debut_signe < 250 ? -2 : -4;
  } else if (alerte.icone == Icone::ATTENTION) {
    f.lun = 2;
    f.yeux = ECARQUILLES;
    f.bouche = 3;
  }
  return true;
}

inline void calcul_pose(Etat suivant, uint32_t t, bool reactions, Costume costume) {
  Animation &a = animation;
  Frame f{};
  f.costume = costume;
  bool au_repos = false;
  if (suivant == Etat::CLAWD) {
    // Les gestes et le réveil sont prioritaires sur la pose d'alerte.
    const auto r = interaction.reaction;
    const bool geste = interaction.appui || r == Reaction::SURSAUT || r == Reaction::CALIN ||
                       r == Reaction::ETOURDI || r == Reaction::REVEIL || r == Reaction::RETOUR_REVEIL;
    if (geste || !pose_alerte(a, f, t)) {
      calcul_reaction(f, t, reactions);
      if (interaction.appui) {
        if (f.yeux == OUVERTS) {
          f.lun = 1;
          f.regard = interaction.regard;
        }
      } else if (interaction.reaction == Reaction::AUCUNE && !pose_alerte(a, f, t)) {
        calcul_repos(a.repos, f, t, reactions);
        au_repos = true;
      }
    }
  } else if (suivant == Etat::ECOUTE) {
    f.lun = 1;
    f.gauche = -1;
    f.ondes = true;
  }
  if (!au_repos) a.repos.actif = false;
  // Un saut d'une case laisse de la place au bonnet sous l'heure.
  if (costume == Costume::NOEL) f.dy = std::max<int8_t>(f.dy, -1);
  a.courante = f;
}

inline constexpr int LARGEUR = 480;
inline constexpr int HAUT_FIN = 64;
inline constexpr int BAS_DEBUT = 424;
inline constexpr int HAUTEUR = 480;
inline constexpr int TEMP_INCONNUE = std::numeric_limits<int>::min();

inline constexpr int BLOC_HEURE = 6, BLOC_DATE = 3;
inline constexpr int LARGEUR_CHIFFRE = 5, HAUTEUR_CHIFFRE = 7;
inline constexpr int PAS_GLYPHE = LARGEUR_CHIFFRE + 1;
inline constexpr int HEURE_X = 24, HEURE_Y = 12;
inline constexpr int CHIFFRES_HEURE = 4, ESPACE_DEUX_POINTS = 2 * BLOC_HEURE;
inline constexpr int DEUX_POINTS_X = HEURE_X + 2 * PAS_GLYPHE * BLOC_HEURE;
inline constexpr int DECALAGE_HEURE = 2;
inline constexpr int HAUT_HEURE_LARGEUR = 224;  // Bord réservé à gauche pour l'heure.
inline constexpr int HEURE_LARGEUR = (CHIFFRES_HEURE * PAS_GLYPHE - 1) * BLOC_HEURE + ESPACE_DEUX_POINTS;
inline constexpr int DATE_X = HEURE_X + HEURE_LARGEUR + 24;
inline constexpr int DATE_Y = HEURE_Y + HAUTEUR_CHIFFRE * (BLOC_HEURE - BLOC_DATE);
inline constexpr int DATE_HAUT = DATE_Y - DECALAGE_HEURE;
inline constexpr int DATE_BAS = DATE_Y + HAUTEUR_CHIFFRE * BLOC_DATE + DECALAGE_HEURE;
inline constexpr int DATE_LARGEUR = (5 * PAS_GLYPHE - 1) * BLOC_DATE;
inline constexpr int HAUT_DATE_LARGEUR = DATE_X + DATE_LARGEUR + DECALAGE_HEURE;
inline constexpr int MINUTEUR_BLOC = 3, MINUTEUR_X = 369, MINUTEUR_Y = DATE_Y;
inline constexpr int MINUTEUR_LARGEUR = (5 * PAS_GLYPHE - 1) * MINUTEUR_BLOC;
static_assert(HEURE_X + HEURE_LARGEUR + DECALAGE_HEURE <= HAUT_HEURE_LARGEUR);
static_assert(HEURE_Y + HAUTEUR_CHIFFRE * BLOC_HEURE + DECALAGE_HEURE <= HAUT_FIN);
static_assert(DATE_BAS <= HAUT_FIN && HAUT_DATE_LARGEUR <= 300);
static_assert(MINUTEUR_X >= 330 && MINUTEUR_X >= HAUT_HEURE_LARGEUR && MINUTEUR_X >= HAUT_DATE_LARGEUR);
static_assert(MINUTEUR_X + MINUTEUR_LARGEUR <= LARGEUR &&
              MINUTEUR_Y + HAUTEUR_CHIFFRE * MINUTEUR_BLOC <= HAUT_FIN);

inline constexpr int TUILE_X = 24, TUILE_LARGEUR = LARGEUR - 2 * TUILE_X;
inline constexpr int TUILE_HAUTEUR = 92, TUILE_PAS = 100, TUILE_Y = 70;
inline constexpr int AJUST_X[] = {25, 135, 245, 355}, AJUST_LARGEUR = 100;
inline constexpr int AJUST_PAS[] = {-5, -1, 1, 5};
inline constexpr int ANNULER_X = 336, ANNULER_LARGEUR = 120, DEMARRER_LARGEUR = 300;
inline constexpr int DENTS_X = 308, DENTS_Y = 80, DENTS_LARGEUR = 148, DENTS_HAUTEUR = 72;
static_assert(TUILE_Y >= HAUT_FIN && TUILE_Y + 2 * TUILE_PAS + TUILE_HAUTEUR <= BAS_DEBUT);
static_assert(DENTS_Y >= TUILE_Y && DENTS_Y + DENTS_HAUTEUR <= TUILE_Y + TUILE_HAUTEUR);

enum class ChoixMenu { RIEN, BUREAU, DEGUISEMENT };

struct Haut {
  int minute = -1;
  int jour = 0, mois = 0;
  bool deux_points = true;
};

struct Bas {
  int temperature = TEMP_INCONNUE;
  int animation = 0;
  Etat etat = Etat::INCONNU;
  bool annonce = false;
  uint32_t revision_alerte = 0;
  bool icone_claire = false;

  bool contenu_change(const Bas &autre) const {
    return temperature != autre.temperature || etat != autre.etat ||
           annonce != autre.annonce || revision_alerte != autre.revision_alerte;
  }
};

struct Tuiles {
  bool lampe = false, minuteur = false;  // `lampe` reste faux : la rangée du milieu est fixe.
  Costume costume = Costume::AUCUN;
  int choix = 5;
};

struct Affichage {
  Haut haut, haut_dessine;
  Bas bas, bas_dessine;
  Etat etat = Etat::INCONNU;
  uint16_t zones = 0;
  bool complet = true;
  bool milieu_invalide = false;
  int minuteur = -1, minuteur_dessine = -1;
  uint32_t brossage = 0, brossage_dessine = 0;
  bool ecran_brossage = false, brossage_invalide = true;
  Tuiles tuiles, tuiles_dessinees;
  uint32_t debut_etat = 0;
  uint32_t t = 0, dernier_perso = 0, dernier_bas = 0u - PAS_BAS;

  bool bas_disponible(uint32_t maintenant) const {
    return maintenant - dernier_bas >= PAS_BAS;
  }
};

inline Affichage affichage;

inline constexpr int PIXEL_CODE = 2;
inline constexpr int LARGEUR_CODE = LARGEUR_CHIFFRE * PIXEL_CODE;
inline constexpr int HAUTEUR_CODE = HAUTEUR_CHIFFRE * PIXEL_CODE;
inline constexpr int PAS_CODE = 17;
inline constexpr int RANGS_CODE = (BAS_DEBUT - HAUT_FIN + PAS_CODE - 1) / PAS_CODE;
static_assert(RANGS_CODE <= 32);

struct ColonneCode {
  uint16_t x, depart;
  uint8_t periode, longueur, vitesse;
};

// Position et phase initiale en pixels, période et traînée en rangs.
inline constexpr ColonneCode COLONNES_CODE[] = {
    {8, 151, 13, 10, 2}, {26, 194, 15, 12, 3}, {46, 154, 12, 10, 2},
    {62, 177, 14, 12, 3}, {81, 51, 15, 12, 3}, {106, 63, 17, 15, 2},
    {130, 38, 14, 11, 3}, {153, 49, 16, 12, 2}, {176, 31, 13, 10, 4},
    {199, 57, 15, 12, 3}, {221, 50, 15, 11, 2}, {245, 36, 17, 12, 3},
    {269, 65, 15, 12, 2}, {289, 46, 14, 10, 3}, {313, 30, 16, 12, 4},
    {338, 58, 18, 14, 2}, {362, 35, 16, 13, 3}, {387, 76, 17, 13, 3},
    {405, 165, 12, 10, 2}, {426, 182, 13, 11, 3}, {449, 146, 13, 10, 2},
    {468, 197, 11, 9, 3},
};
inline constexpr size_t NB_COLONNES_CODE = sizeof(COLONNES_CODE) / sizeof(COLONNES_CODE[0]);

struct PluieMatrix {
  uint16_t phase[NB_COLONNES_CODE]{};
  uint32_t dernier_pas = 0;
  bool en_pause = true;

  constexpr PluieMatrix() {
    for (size_t i = 0; i < NB_COLONNES_CODE; i++) phase[i] = COLONNES_CODE[i].depart;
  }

  bool avancer(uint32_t t) {
    if (en_pause) {
      dernier_pas = t;
      en_pause = false;
      return false;
    }
    if (t - dernier_pas < PAS_PLUIE) return false;
    dernier_pas = t;
    for (size_t i = 0; i < NB_COLONNES_CODE; i++) {
      const auto &c = COLONNES_CODE[i];
      phase[i] = (phase[i] + c.vitesse) % (2 * c.periode * PAS_CODE);
    }
    return true;
  }

  int origine(size_t i) const { return HAUT_FIN + phase[i] % PAS_CODE; }
};

inline PluieMatrix pluie_matrix;

struct MasqueCode {
  // Un bit par rang de chiffre, calculé à partir des rectangles de la pose.
  uint32_t rangs[NB_COLONNES_CODE]{};

  void couvrir(int x, int y, int w, int h) {
    constexpr int marge = 6;
    for (size_t i = 0; i < NB_COLONNES_CODE; i++) {
      const int cx = COLONNES_CODE[i].x;
      if (cx + LARGEUR_CODE <= x - marge || cx >= x + w + marge) continue;
      const int origine = pluie_matrix.origine(i);
      const int haut = std::max(0, y - marge - origine - HAUTEUR_CODE + 1);
      const int bas = y + h + marge - 1 - origine;
      for (int rang = (haut + PAS_CODE - 1) / PAS_CODE;
           rang < RANGS_CODE && rang * PAS_CODE <= bas; rang++) {
        rangs[i] |= uint32_t(1) << rang;
      }
    }
  }
};

template<typename Rectangle>
inline void dessiner_bonnet(const Rectangle &b, int front, float dy) {
  // Trois cases au repos. Le bonnet se tasse à la limite de PERSO pendant un saut.
  constexpr float hauteur = 3;
  const float echelle = std::min(1.0f, (front + dy - float(HAUT_FIN) / T) / hauteur);
  const auto c = [&b, front, echelle](float x, float y, float w, float h, esphome::Color col) {
    b(x, front + y * echelle, w, h * echelle, col);
  };
  c(4, -3, 3, 0.5, C.NOEL);
  c(3.5, -2.5, 5, 0.5, C.NOEL);
  c(4.5, -2, 5, 0.5, C.NOEL);
  c(5, -1.5, 5.5, 1, C.NOEL);
  c(4.75, -0.5, 6, 0.5, C_PERSO.BLANC);
  c(3.25, -2, 1, 0.75, C_PERSO.BLANC);
  c(3.5, -2.25, 0.5, 1.25, C_PERSO.BLANC);
}

inline void dessiner_glyphe(esphome::display::Display &it, int x, int y, char c, int bloc,
                            int colonnes, esphome::Color col);
inline void dessiner_pluie(esphome::display::Display &it, const MasqueCode &masque);

// Motifs des planches C et D, un caractère par pixel et un point pour le fond.
inline constexpr char LUNETTES_NOIRES[][45] = {
    "..AACCCCCCCCCCCCCAA......AACCCCCCCCCCCCCAA..",
    "AAAAAAAAAAAAAAAAAAAACCCCAAAAAAAAAAAAAAAAAAAA",
    "AAABDDDDBBBBBBBBBBBAAAAAAABDDDDBBBBBBBBBBBAA",
    ".AADDEEEEEBBBBBBBBBBAAAAAADDEEEEEBBBBBBBBBBA",
    ".AABBBBBBBBBBBBBBBBBA...AABBBBBBBBBBBBBBBBBA",
    ".AABBBBBBBBBBBBBBBBBA...AABBBBBBBBBBBBBBBBBA",
    ".AABBBBBBBBBBBBBBBBBA...AABBBBBBBBBBBBBBBBBA",
    ".AABBBBBBBBBBBBBBBBAA...AABBBBBBBBBBBBBBBBAA",
    "..AAAABBBBBBBBBBBBAA.....AAAABBBBBBBBBBBBAA.",
    "...AAAAAAAAAAAAAAAA.......AAAAAAAAAAAAAAAA..",
    ".....AAAAAAAAAAA............AAAAAAAAAAA.....",
};
inline constexpr esphome::Color C_LUNETTES_NOIRES[] = {
    {0x12, 0x0e, 0x12}, {0x18, 0x15, 0x1a}, {0x2b, 0x22, 0x29}, {0x71, 0x6e, 0x73}, {0x41, 0x3a, 0x43},
};

inline constexpr uint8_t REFLETS_YEUX[][3] = {
    // Dans l'ordre de Yeux, cinq pixels par ligne.
    {0, 0, 0}, {0, 31, 0}, {14, 17, 14},
    {14, 17, 0}, {0, 17, 14}, {17, 10, 17}, {10, 21, 0},
};

inline constexpr esphome::Color CONTOUR_ORANGE{0x63, 0x2d, 0x25};
inline constexpr esphome::Color ECLAT_ORANGE{0xff, 0xc0, 0x80};
inline constexpr esphome::Color C_BRAS[] = {
    CONTOUR_ORANGE, C_PERSO.OMBRE, C_PERSO.CORPS, C_PERSO.CLAIR, ECLAT_ORANGE,
};
inline constexpr char PINCE[][9] = {
    "...AAAA.",
    "...AEEC.",
    "AAAACDCA",
    "ABA.ADDA",
    "ABAAACCC",
    "ABCBBCCC",
    "ABCDDDDC",
    "AABCCCCC",
    ".AABBBBB",
    "..AAAAAA",
};
inline constexpr char POIGNET[][6] = {
    ".....",
    ".AAAA",
    ".ACCC",
    "AACCC",
    "ACCCC",
    "CCCCC",
    "CCCCC",
    "BBBBB",
    "AAAAA",
};
inline constexpr char COUDE_TASSE[][16] = {
    ".......ACCBAA..",
    "......ACCCCBA..",
    "......ADCCCBA..",
    ".....AADCCCBA..",
    ".....DDCCCBA...",
    "....AACCCBBA...",
    "AAAAACCCCBA....",
    "ACCCCCCCBA.....",
    "ACCCCCCBBA.....",
    "ACCCCCBBA......",
    "ACCCCBBA.......",
    "ABBBBA.........",
    "AAAAA..........",
};
inline constexpr char TASSE[][13] = {
    "DFFFEEEED...",
    "DDDDDDDDD...",
    "ACCCCCBBAAA.",
    "ADCCCCBBABAA",
    "ADCCCCBBABAA",
    "ADCCCCBBABAA",
    "ACCCCCBBAAAA",
    "ABCCCBBBAAA.",
    "AABBCBBAAA..",
    ".AAAAABA....",
    "..AAAAA.....",
};
inline constexpr esphome::Color C_TASSE[] = {
    {0x29, 0x36, 0x38}, {0x65, 0x84, 0x86}, {0xb2, 0xce, 0xca}, {0xe9, 0xee, 0xe1},
    {0x62, 0x3b, 0x2d}, {0xa4, 0x6b, 0x47},
};

template<typename Rectangle, size_t Lignes, size_t Colonnes>
inline void dessiner_motif(const Rectangle &rect, int x, int y, int px, int py,
                           const char (&motif)[Lignes][Colonnes], const esphome::Color *palette) {
  for (size_t j = 0; j < Lignes; j++) {
    for (int i = 0; motif[j][i];) {
      const int debut = i;
      const char teinte = motif[j][i++];
      while (motif[j][i] == teinte) i++;
      if (teinte != '.') rect(x + debut * px, y + int(j) * py, (i - debut) * px, py, palette[teinte - 'A']);
    }
  }
}

template<typename Rectangle>
inline void dessiner_bras(const Rectangle &rect, int bord, int y, int decalage, bool gauche, bool etirement) {
  constexpr int pixel = T / 4;
  const int dispo = gauche ? bord : LARGEUR - bord;
  const int colonnes = etirement ? 12 : 11;  // Pince et poignet visibles.
  const int pas = std::min(pixel, dispo / colonnes);
  const int x = gauche ? bord - colonnes * pas : bord + colonnes * pas;
  const auto orienter = [&rect, x, gauche](int i, int y, int w, int h, esphome::Color col) {
    rect(gauche ? x + i : x - i - w, y, w, h, col);
  };
  const int pente = std::clamp(-decalage, -1, 2);
  const auto plier = [&orienter, pas, pente](int i, int y, int w, int h, esphome::Color col) {
    if (pente == 0) {
      orienter(i, y, w, h, col);
    } else {
      for (int j = i; j < i + w; j += pas) {
        orienter(j, y + (j / pas - 8) * pente * h, pas, h, col);
      }
    }
  };
  const int haut = y + decalage * T - 3 * pixel;
  dessiner_motif(orienter, 0, haut, pas, pixel, PINCE, C_BRAS);
  dessiner_motif(plier, 8 * pas, haut, pas, pixel, POIGNET, C_BRAS);
}

template<typename Rectangle>
inline void dessiner_cafe(const Rectangle &rect, int x, int y, uint8_t vapeur) {
  constexpr int pas = 5, pixel = T / 4;  // L'anse reste à l'écran avec le décalage latéral.
  dessiner_motif(rect, x, y + 11 * pixel, pas, pixel, COUDE_TASSE, C_BRAS);
  dessiner_motif(rect, x + 3 * pas, y, pas, pixel, TASSE, C_TASSE);
  // Les doigts passent devant le bas du mug.
  dessiner_motif(rect, x + 3 * pas, y + 6 * pixel, pixel, pixel, PINCE, C_BRAS);
  const int montee = 3 * (vapeur - 1);
  rect(x + 25, y - 21 - montee, 10, 12, {0xae, 0xbf, 0xba});
  rect(x + 35, y - 39 - montee, 10, 12, {0xd5, 0xdf, 0xd7});
}

inline constexpr int POSITIONS_BROSSE[] = {0, 12, 18, 12, 0, -12, -18, -12};

inline uint32_t pas_brosse(uint32_t ecoule) {
  return ecoule * FREQUENCE_BROSSE / 1000;
}

inline int position_brosse(uint32_t ecoule) {
  return POSITIONS_BROSSE[pas_brosse(ecoule) % std::size(POSITIONS_BROSSE)];
}

inline void dessiner_perso(esphome::display::Display &it, const Frame &f, int inclinaison = 0, bool bravo = false,
                           int dx_brosse = 0) {
  const bool matrix = f.costume == Costume::MATRIX;
  const auto &couleurs = C_PERSO;
  const int X = f.dx, Y = f.dy;
  const auto px = [X, &f](float x) { return int(std::round((x + X) * T)) + f.dx_px; };
  const auto py = [Y, &f](float y) { return int(std::round((y + Y) * T)) + f.dy_px; };
  MasqueCode masque;
  const auto clip = inclinaison ? it.get_clipping() : esphome::display::Rect{};
  const auto rect = [&it, &masque, matrix, &clip](int x, int y, int w, int h, esphome::Color couleur) {
    if (clip.is_set()) {
      const int haut = std::max(y, int(clip.y)), bas = std::min(y + h, int(clip.y2()));
      if (bas <= haut) return;
      it.filled_rectangle(x, haut, w, bas - haut, couleur);
    } else {
      it.filled_rectangle(x, y, w, h, couleur);
    }
    if (matrix) masque.couvrir(x, y, w, h);
  };
  const int amplitude = std::abs(dx_brosse);
  const int tdy = amplitude >= 18 ? 12 : amplitude >= 12 ? 6 : 0;
  const auto b = [&rect, &px, &py, inclinaison, tdy](float x, float y, float w, float h, esphome::Color couleur) {
    const int gauche = px(x), haut = py(y);
    const int largeur = int(std::round(w * T)), bas = haut + int(std::round(h * T));
    if (!inclinaison || y >= 15) {
      rect(gauche, haut, largeur, bas - haut, couleur);
      return;
    }
    // Décaler la tête par bandes de 6 px et atténuer la pente en arrivant aux pattes.
    constexpr int bande = T / 4;
    for (int colonne = gauche; colonne < gauche + largeur; colonne += bande) {
      const int pente = inclinaison * int(std::floor((colonne + bande / 2 - 228) / 66.0f + 0.5f));
      const auto pencher = [pente, tdy](int ligne) {
        ligne += tdy;
        const float poids = std::clamp((360 - ligne) / 36.0f, 0.0f, 1.0f);
        return ligne + bande * int(std::floor(pente * poids + 0.5f));
      };
      const int debut = pencher(haut);
      rect(colonne, debut, std::min(bande, gauche + largeur - colonne), pencher(bas) - debut, couleur);
    }
  };

  // L'ombre reste au sol et se resserre avec la hauteur du saut.
  const int retrait = std::max(0, -Y * T - std::max<int>(0, f.dy_px));
  const int ombre_x = X * T + f.dx_px + retrait;
  rect(4 * T + ombre_x, 17 * T, 11 * T - 2 * retrait, T / 2, C.SOL);
  rect(3 * T + ombre_x + T / 2, 17 * T, 12 * T - 2 * retrait, T / 4, C.SOL);

  const int front = 6 + f.ecrase;
  if (f.drapeau) {
    constexpr float tissu = 2.25f, bord = 0.5f, hauteur = 2.5f;
    // Le mât passe derrière la pince. L'ondulation garde un quart de case sous l'heure.
    const int droit = 12 + f.droit;
    const float mat = std::min(16.5f, float(LARGEUR - px(0)) / T - tissu - bord);
    const float haut = std::max(droit - 7.0f, float(HAUT_FIN - py(0)) / T + 0.25f);
    b(mat, haut, 0.25f, droit + 0.75f - haut, C_PERSO.LUN);
    b(mat, haut, tissu, hauteur, C.DRAP);
    b(mat + tissu, haut + 0.25f * f.fanion, bord, hauteur, C.DRAP);
    const int centre = px(mat + (tissu + bord) / 2);
    const auto inscription = [&it, centre](const char *mot, int y) {
      constexpr int bloc = 2;
      const int largeur = (int(std::strlen(mot)) * PAS_GLYPHE - 1) * bloc;
      int x = centre - largeur / 2;
      for (; *mot; ++mot, x += PAS_GLYPHE * bloc) {
        dessiner_glyphe(it, x, y, *mot, bloc, LARGEUR_CHIFFRE, C.TXTF);
      }
    };
    inscription("VIBE", py(haut) + 9);
    inscription("CODER", py(haut) + 34);
  }
  const auto pince = [&rect, inclinaison, dx_brosse](int x, int y, int w, int h, esphome::Color col) {
    rect(x - inclinaison * dx_brosse, y, w, h, col);
  };
  if (inclinaison < 0) dessiner_bras(pince, px(3), py(12), f.gauche, true, f.etirement);
  else dessiner_bras(rect, px(3), py(12), f.gauche, true, f.etirement);
  if (f.vapeur) {
    dessiner_cafe(rect, px(15.75f), py(7.75f), f.vapeur);
  } else if (inclinaison > 0) {
    dessiner_bras(pince, px(16), py(12), f.droit, false, f.etirement);
  } else {
    dessiner_bras(rect, px(16), py(12), f.droit, false, f.etirement);
  }
  for (int x = 4; x <= 13; x += 3) {
    b(x, 15, 2, 2, couleurs.OMBRE);
    b(x, 15, 2, 1.5, couleurs.CORPS);
  }
  b(3, front, 13, 15 - front, couleurs.OMBRE);
  b(3, front, 13, 0.25, CONTOUR_ORANGE);
  b(3, front + 0.25, 0.25, 14.75 - front, CONTOUR_ORANGE);
  b(15.75, front + 0.25, 0.25, 14.75 - front, CONTOUR_ORANGE);
  b(3, 14.75, 13, 0.25, CONTOUR_ORANGE);
  b(3.5, front + 0.5, 12, 14 - front, couleurs.CORPS);
  b(3.5, front + 0.25, 12, 0.25, couleurs.CLAIR);
  b(3.5, front + 0.25, 5.25, 0.25, ECLAT_ORANGE);
  b(3.5, front + 0.5, 0.75, 0.25, couleurs.CLAIR);
  b(3.5, front + 0.75, 0.5, 0.5, couleurs.CLAIR);
  b(3.5, front + 1.25, 0.25, 0.75, couleurs.CLAIR);
  b(15.25, front + 1.75, 0.25, 1.75, couleurs.OMBRE);
  b(15, front + 2.25, 0.25, 1.25, couleurs.OMBRE);
  if (matrix) {
    // Laisser aussi libres les intervalles entre les pattes.
    masque.couvrir(px(4), py(15), 11 * T, 2 * T);
  }
  if (f.costume == Costume::NOEL) dessiner_bonnet(b, front, Y + float(f.dy_px) / T);

  // Visage et lunettes sont ancrés aux joues, y compris quand le front se tasse.
  if (!matrix && f.lun) {
    for (int x = 6; x <= 12; x += 6) {
      if (f.yeux == FERMES) {
        b(x - 0.25, 12.25, 1.5, 0.5, couleurs.OEIL);
      } else if (f.yeux == ECARQUILLES) {
        b(x - 0.25, 11.25, 1.5, 1.5, couleurs.OEIL);
        b(x, 11.5, 0.5, 0.5, couleurs.BLANC);
      } else if (f.yeux == ARCS) {
        b(x - 0.25, 12.5, 0.25, 0.5, couleurs.OEIL);
        b(x, 12.25, 1, 0.25, couleurs.OEIL);
        b(x + 1, 12.5, 0.25, 0.5, couleurs.OEIL);
      } else if (f.yeux == BAISSES) {
        b(x - 0.25, 12.25, 1.5, 0.25, couleurs.OEIL);
        b(x + 0.25, 12.5, 0.75, 0.5, couleurs.OEIL);
      } else if (f.yeux == PLISSES) {
        b(x - 0.25, 12.5, 1.5, 0.25, couleurs.OEIL);
        b(x, 12.75, 1, 0.25, couleurs.OEIL);
      } else if (f.yeux == CROIX) {
        for (int i = -2; i <= 2; i++) {
          b(x + 0.25 + 0.25 * i, 11.75 + 0.25 * i, 0.25, 0.25, couleurs.OEIL);
          b(x + 0.25 - 0.25 * i, 11.75 + 0.25 * i, 0.25, 0.25, couleurs.OEIL);
        }
      } else {
        b(x + 0.125 + 0.5 * f.regard, 12.25, 0.75, 0.75, couleurs.OEIL);
      }
    }
  }
  const int barre = 10 - f.lun;
  if (matrix) {
    dessiner_motif(rect, px(4.5f), py(barre), T / 4, T / 4, LUNETTES_NOIRES, C_LUNETTES_NOIRES);
    // Le reflet de la planche C reste en place. Un reflet discret indique l'expression.
    for (float x : {6.75f, 12.5f}) {
      if (f.yeux == OUVERTS && f.regard) {
        b(x + 0.5f * f.regard, barre + 1.5, 0.5, 0.25, C_LUNETTES_NOIRES[4]);
      }
      for (int ligne = 0; ligne < 3; ++ligne) {
        const uint8_t bits = REFLETS_YEUX[f.yeux][ligne];
        for (int col = 0; col < 5; ++col) {
          if (bits & (16 >> col)) {
            b(x + 0.25f * col, barre + 1.25f + 0.25f * ligne, 0.25, 0.25, C_LUNETTES_NOIRES[4]);
          }
        }
      }
    }
  } else {
    b(4, barre, 11, 1, couleurs.LUN);
    b(4, barre, 11, 0.25, couleurs.VERRE);
    for (int x = 4; x <= 10; x += 6) {
      b(x, barre + 1, 5, 1, couleurs.LUN);
      b(x + 1, barre + 2, 3, 1, couleurs.LUN);
      b(x + 0.5, barre + 1, 0.5, 0.5, couleurs.BLANC);
      b(x + 1, barre + 1.5, 0.5, 0.5, couleurs.BLANC);
      b(x + 1.5, barre + 2, 0.5, 0.5, couleurs.REFLET);
    }
  }
  b(5, 13.25, 1.5, 0.5, couleurs.JOUE);
  b(12.5, 13.25, 1.5, 0.5, couleurs.JOUE);
  if (bravo) {
    b(8, 13.25, 3, 0.5, couleurs.OEIL);
    b(8.5, 13.75, 2, 0.75, couleurs.OEIL);
    b(8.5, 13.25, 2, 0.5, couleurs.BLANC);
  } else if (inclinaison) {
    b(8.25, 13, 2.5, 1.5, couleurs.OEIL);
  } else if (f.bouche == 1) {
    b(8.5, 13, 2, 1.5, couleurs.OEIL);
    b(9, 14, 1, 0.5, couleurs.LANGUE);
  } else if (f.bouche == 2) {
    b(9, 13.75, 1, 0.5, couleurs.OEIL);
  } else if (f.bouche == 3) {
    b(8.75, 13.5, 1.5, 0.25, couleurs.OEIL);
    b(8.25, 13.75, 0.5, 0.5, couleurs.OEIL);
    b(10.25, 13.75, 0.5, 0.5, couleurs.OEIL);
  } else {
    b(8.25, 13.25, 0.5, 0.5, couleurs.OEIL);
    b(10.25, 13.25, 0.5, 0.5, couleurs.OEIL);
    b(8.75, 13.75, 1.5, 0.5, couleurs.OEIL);
  }

  if (f.ondes) {
    b(1.75, 7.5, 0.5, 0.25, C.ONDE);
    b(1.5, 8.25, 0.5, 0.25, C.ONDE);
    b(16.75, 7.5, 0.5, 0.25, C.ONDE);
    b(17, 8.25, 0.5, 0.25, C.ONDE);
  }

  if (f.effet == Reaction::REVEIL) {
    b(1.25, 7, 0.75, 0.25, C.ONDE);
    b(1.5, 8, 0.75, 0.25, C.ONDE);
    b(17, 7, 0.75, 0.25, C.ONDE);
    b(16.75, 8, 0.75, 0.25, C.ONDE);
  } else if (f.effet == Reaction::CALIN) {
    for (int i = 0; i < 3; i++) {
      const float x = 4 + 4 * i, y = 8 - 0.5f * (i % 2) - 0.25f * f.pas;
      b(x + 0.25, y, 0.5, 0.25, C.COEUR);
      b(x + 1, y, 0.5, 0.25, C.COEUR);
      b(x, y + 0.25, 1.75, 0.5, C.COEUR);
      b(x + 0.25, y + 0.75, 1.25, 0.25, C.COEUR);
      b(x + 0.5, y + 1, 0.75, 0.25, C.COEUR);
      b(x + 0.75, y + 1.25, 0.25, 0.25, C.COEUR);
    }
  } else if (f.effet == Reaction::ETOURDI || f.effet == Reaction::CONTENT) {
    // Positions en cases autour du front. Le décalage de trois pas espace les étoiles.
    constexpr float orbite[][2] = {{5, 5}, {7, 4}, {9, 4}, {11, 4}, {13, 5}, {11, 5.5f}, {9, 5.75f}, {7, 5.5f}};
    for (int i = 0; i < 3; i++) {
      const bool tourne = f.effet == Reaction::ETOURDI;
      const auto &p = orbite[tourne ? (f.pas + 3 * i) % 8 : i * 2];
      const auto col = (i + f.pas) % 2 ? C.REFLEXION : C.TEXTE;
      b(p[0] + 0.25, p[1] - 0.25, 0.25, 0.75, col);
      b(p[0], p[1], 0.75, 0.25, col);
    }
  } else if (f.effet == Reaction::TRISTE) {
    const float y = 12.75f + 0.25f * f.pas;
    b(13.5, y, 0.25, 0.25, C.REPONSE);
    b(13.25, y + 0.25, 0.75, 0.5, C.REPONSE);
    b(13.5, y + 0.75, 0.25, 0.25, C.REPONSE);
  }
  if (matrix) dessiner_pluie(it, masque);
}

// Sept rangées, bit de gauche en premier. Les chiffres occupent cinq colonnes.
inline constexpr uint8_t PIXELS[][7] = {
    {14, 17, 19, 21, 25, 17, 14},  // 0
    {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},
    {31, 16, 16, 30, 1, 1, 30},
    {14, 16, 16, 30, 17, 17, 14},
    {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 1, 14},
    {0, 0, 0, 31, 0, 0, 0},      // -
    {0, 0, 0, 0, 0, 0, 1},       // .
    {7, 5, 7, 0, 0, 0, 0},       // °
    {0, 0, 4, 0, 4, 0, 0},       // :
    {4, 4, 4, 4, 4, 0, 4},       // !
    {4, 4, 8, 0, 0, 0, 0},       // '
    {14, 17, 1, 2, 4, 0, 4},     // ?
    {0, 0, 0, 0, 0, 0, 0},       // espace
    {1, 1, 2, 4, 8, 16, 16},     // /
    {14, 17, 17, 31, 17, 17, 17},  // A
    {30, 17, 17, 30, 17, 17, 30},
    {14, 17, 16, 16, 16, 17, 14},
    {30, 17, 17, 17, 17, 17, 30},
    {31, 16, 16, 30, 16, 16, 31},
    {31, 16, 16, 30, 16, 16, 16},
    {14, 17, 16, 23, 17, 17, 15},
    {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14},
    {7, 2, 2, 2, 2, 18, 12},
    {17, 18, 20, 24, 20, 18, 17},
    {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17},
    {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14},
    {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13},
    {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},
    {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14},
    {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 27, 17},
    {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},
    {31, 1, 2, 4, 8, 16, 31}  // Z
};
inline int glyphe(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'Z') return 19 + c - 'A';
  constexpr char symboles[] = "-.o:!'? /";  // 'o' désigne le petit symbole degré.
  const char *symbole = c ? std::strchr(symboles, c) : nullptr;
  return symbole ? 10 + (symbole - symboles) : 16;
}

inline void dessiner_pixels(esphome::display::Display &it, int x, int y, const uint8_t *lignes,
                            int bloc, int colonnes, esphome::Color col) {
  for (int ligne = 0; ligne < HAUTEUR_CHIFFRE; ligne++) {
    for (int i = 0; i < colonnes;) {
      const int debut = i;
      while (i < colonnes && (lignes[ligne] & (1 << (colonnes - 1 - i)))) i++;
      if (i > debut) it.filled_rectangle(x + debut * bloc, y + ligne * bloc, (i - debut) * bloc, bloc, col);
      else i++;
    }
  }
}

inline void dessiner_glyphe(esphome::display::Display &it, int x, int y, char c, int bloc,
                            int colonnes, esphome::Color col) {
  dessiner_pixels(it, x, y, PIXELS[glyphe(c)], bloc, colonnes, col);
}

inline void dessiner_texte_pixel(esphome::display::Display &it, int x, int y, const char *texte,
                                 int bloc, esphome::Color col) {
  for (; *texte; ++texte, x += PAS_GLYPHE * bloc) {
    if (*texte == '+') {  // Pas dans la police : croix de 5 cases.
      it.filled_rectangle(x, y + 3 * bloc, LARGEUR_CHIFFRE * bloc, bloc, col);
      it.filled_rectangle(x + 2 * bloc, y + 1 * bloc, bloc, 5 * bloc, col);
      continue;
    }
    dessiner_glyphe(it, x, y, *texte, bloc, LARGEUR_CHIFFRE, col);
  }
}

inline void dessiner_texte_centre(esphome::display::Display &it, int centre, int y, const char *texte,
                                 int bloc, esphome::Color col) {
  const int largeur = (int(std::strlen(texte)) * PAS_GLYPHE - 1) * bloc;
  dessiner_texte_pixel(it, centre - largeur / 2, y, texte, bloc, col);
}

inline void dessiner_bouton(esphome::display::Display &it, int x, int y, int l, int h, const char *texte,
                            int bloc, esphome::Color col) {
  it.rectangle(x, y, l, h, col);
  dessiner_texte_centre(it, x + l / 2, y + (h - 7 * bloc) / 2, texte, bloc, col);
}

inline void dessiner_dent(esphome::display::Display &it, int x, int y, int pas) {
  constexpr char motif[][10] = {
      ".AA...AA.", "AAAAAAAAA", "AAAAAAAAA", "AACAAAABA",
      "AAAAAAABA", ".AAABABA.", ".AAB.BBA.", "..A...A..",
  };
  constexpr esphome::Color palette[] = {C_PERSO.BLANC, C.REPONSE, C.TEXTE};
  const auto rect = [&it](int x, int y, int w, int h, esphome::Color col) {
    it.filled_rectangle(x, y, w, h, col);
  };
  dessiner_motif(rect, x, y, pas, pas, motif, palette);
}

inline void dessiner_tuile(esphome::display::Display &it, Zone zone, const Tuiles &tuiles) {
  const int index = zone == MENU_MINUTEUR ? 0 : zone == MENU_BUREAU ? 1 : 2;
  const int y = TUILE_Y + index * TUILE_PAS;
  const int debut = index == 0 ? HAUT_FIN : y;
  const int fin = index == 2 ? BAS_DEBUT : y + TUILE_PAS;
  it.filled_rectangle(0, debut, LARGEUR, fin - debut, C.BG);
  if (zone == MENU_MINUTEUR) {
    char texte[8];
    std::snprintf(texte, sizeof(texte), "%d MIN", tuiles.choix);
    dessiner_texte_centre(it, 140, y + 18, texte, 8, C.TEXTE);
    it.rectangle(DENTS_X, DENTS_Y, DENTS_LARGEUR, DENTS_HAUTEUR, C.REFLEXION);
    dessiner_dent(it, 318, 102, 3);
    dessiner_texte_centre(it, 395, 96, "DENTS", 3, C.REFLEXION);
    dessiner_texte_centre(it, 395, 122, "2 MIN", 3, C.DISCRET);
  } else if (zone == MENU_BUREAU) {
    for (size_t i = 0; i < std::size(AJUST_X); ++i) {
      char texte[4];
      std::snprintf(texte, sizeof(texte), "%+d", AJUST_PAS[i]);
      dessiner_bouton(it, AJUST_X[i], y, AJUST_LARGEUR, TUILE_HAUTEUR, texte, 5, C.REPONSE);
    }
  } else {
    dessiner_bouton(it, TUILE_X, y, tuiles.minuteur ? DEMARRER_LARGEUR : TUILE_LARGEUR, TUILE_HAUTEUR,
                    "DEMARRER", 5, C.ECOUTE);
    if (tuiles.minuteur) dessiner_bouton(it, ANNULER_X, y, ANNULER_LARGEUR, TUILE_HAUTEUR, "STOP", 4, C.COEUR);
    dessiner_bouton(it, TUILE_X, y + TUILE_PAS + 2, TUILE_LARGEUR, 44, "RETOUR", 3, C.DISCRET);
  }
}

inline void dessiner_brosse(esphome::display::Display &it, bool droite, bool bas, int niveau, int dx) {
  const auto pixel = [&it, droite](int x, int y, int w, int h, esphome::Color col) {
    it.filled_rectangle(droite ? 456 - x - w : x, y, w, h, col);
  };
  const auto brosse = [&pixel, dx](int x, int y, int w, int h, esphome::Color col) {
    pixel(x + dx, y, w, h, col);
  };
  const auto mousse = [&pixel](int x, int y, int pas) {
    constexpr char motif[][5] = {".AA.", "AAAA", ".AAB"};
    constexpr esphome::Color palette[] = {C_PERSO.BLANC, C.REPONSE};
    dessiner_motif(pixel, x, y, pas, pas, motif, palette);
  };
  mousse(204, 318, 6);
  mousse(222, 312, 6);
  if (niveau >= 2) mousse(234, 321, 6);
  if (niveau >= 3) {
    mousse(198, 330, 6);
    pixel(192, 306, 6, 6, C_PERSO.BLANC);
  }
  if (niveau >= 4) {
    mousse(246, 324, 6);
    mousse(222, 342, 4);
    pixel(270, 314, 6, 6, C.REPONSE);
  }

  const int prise = bas ? 294 : 270;
  const int bout = bas ? 342 : 306;
  for (int x = 24; x < 192; x += 6) {
    constexpr int longueur = 192 - 24, pas = 6;
    const int y = prise + pas * (((bout - prise) * (x - 24) + longueur * pas / 2) / (longueur * pas));
    brosse(x, y, 6, 18, C.PISTE);
    brosse(x, y, 6, 12, C.ECOUTE);
  }
  brosse(186, bout, 48, 12, C.REPONSE);
  brosse(192, bout, 36, 6, C.ECOUTE);
  const int poils = bas ? bout - 12 : bout + 12;
  for (int x = 192; x < 234; x += 12) brosse(x, poils, 6, 12, C_PERSO.BLANC);
  brosse(30, prise, 12, 12, C_PERSO.CORPS);
  brosse(30, prise, 12, 6, C_PERSO.CLAIR);
}

inline void dessiner_bulles_de_savon(esphome::display::Display &it, bool droite, uint32_t ecoule) {
  const double t = ecoule / 1000.0;
  for (int k = 0; k < 5; ++k) {
    const double cycle = t * 0.7 + k * 0.2;
    const double phase = cycle - std::floor(cycle);
    const int taille = phase < 0.5 ? 12 : 18;
    const int position = 190 + k * 24 + 6 * int(std::floor(std::sin(t * 5 + k * 2) + 0.5));
    const int x = droite ? 456 - position - taille : position;
    const int y = 6 * int(std::floor((330 - phase * 190) / 6 + 0.5));
    it.filled_rectangle(x, y, taille, 3, C.REPONSE);
    it.filled_rectangle(x, y + taille - 3, taille, 3, C.REPONSE);
    it.filled_rectangle(x, y, 3, taille, C.REPONSE);
    it.filled_rectangle(x + taille - 3, y, 3, taille, C.REPONSE);
    it.filled_rectangle(x + 6, y + 6, 3, 3, C_PERSO.BLANC);
  }
}

inline constexpr int BROSSAGE_TEMPS_Y = 30, BROSSAGE_TEMPS_BLOC = 12, BROSSAGE_CONSIGNE_Y = 434;

inline void dessiner_temps_brossage(esphome::display::Display &it, uint32_t ecoule) {
  const int reste = (DUREE_BROSSAGE - ecoule + 999) / 1000;
  const char texte[] = {'0', char('0' + reste / 60), ':',
                        char('0' + (reste % 60) / 10), char('0' + reste % 10), '\0'};
  dessiner_texte_centre(it, LARGEUR / 2, BROSSAGE_TEMPS_Y, texte, BROSSAGE_TEMPS_BLOC,
                        reste <= 10 ? C.NOEL : C.TEXTE);
}

inline void dessiner_perso_brossage(esphome::display::Display &it, uint32_t ecoule) {
  const int zone = ecoule / DUREE_ZONE_DENTS;
  const bool droite = zone % 2, bas = zone >= 2;
  const int dx = position_brosse(ecoule);
  Frame pose{};
  if (!bas) {
    if (droite) pose.droit = -1;
    else pose.gauche = -1;
  }
  dessiner_perso(it, pose, droite ? 1 : -1, false, dx);
  const int niveau = 1 + ecoule % DUREE_ZONE_DENTS / (DUREE_ZONE_DENTS / 4);
  dessiner_brosse(it, droite, bas, niveau, dx);
  dessiner_bulles_de_savon(it, droite, ecoule);
}

inline bool changement_dents(uint32_t ecoule) {
  return ecoule >= DUREE_ZONE_DENTS && ecoule % DUREE_ZONE_DENTS < DUREE_CHANGEMENT_DENTS;
}

inline void dessiner_consigne_brossage(esphome::display::Display &it, uint32_t ecoule) {
  constexpr const char *zones[] = {"HAUT GAUCHE", "HAUT DROITE", "BAS GAUCHE", "BAS DROITE"};
  const bool changement = changement_dents(ecoule);
  dessiner_texte_centre(it, LARGEUR / 2, BROSSAGE_CONSIGNE_Y,
                       changement ? "ON CHANGE !" : zones[ecoule / DUREE_ZONE_DENTS], 5,
                       changement ? C.REFLEXION : C.ECOUTE);
}

inline void dessiner_bravo(esphome::display::Display &it) {
  constexpr int bloc = 9, y = 30;
  constexpr char texte[] = "Bravo !";
  constexpr uint8_t minuscules[][HAUTEUR_CHIFFRE] = {
      {0, 0, 22, 25, 16, 16, 16}, {0, 0, 12, 2, 14, 18, 15},
      {0, 0, 17, 17, 17, 10, 4}, {0, 0, 14, 17, 17, 17, 14},
  };
  int x = (LARGEUR - (int(std::strlen(texte)) * PAS_GLYPHE - 1) * bloc + 1) / 2;
  for (size_t i = 0; texte[i]; ++i, x += PAS_GLYPHE * bloc) {
    if (i > 0 && i <= std::size(minuscules)) {
      dessiner_pixels(it, x, y, minuscules[i - 1], bloc, LARGEUR_CHIFFRE, C.TEXTE);
    } else {
      dessiner_glyphe(it, x, y, texte[i], bloc, LARGEUR_CHIFFRE, C.TEXTE);
    }
  }
  Frame pose{};
  pose.gauche = -2;
  pose.droit = -3;
  dessiner_perso(it, pose, 0, true);
  dessiner_dent(it, 374, 124, 9);
  const auto etincelle = [&it](int x, int y, int pas) {
    it.filled_rectangle(x + 2 * pas, y, pas, 5 * pas, C.REFLEXION);
    it.filled_rectangle(x, y + 2 * pas, 5 * pas, pas, C.REFLEXION);
    it.filled_rectangle(x + 2 * pas, y + 2 * pas, pas, pas, C_PERSO.BLANC);
  };
  etincelle(352, 101, 4);
  etincelle(435, 95, 6);
  etincelle(440, 211, 4);
  etincelle(87, 112, 4);
}

inline void dessiner_pluie(esphome::display::Display &it, const MasqueCode &masque) {
  constexpr esphome::Color teintes[] = {
      {0x0a, 0x25, 0x15}, {0x12, 0x53, 0x2e}, {0x20, 0x8c, 0x49}, {0x40, 0xcf, 0x6a}, {0xeb, 0xff, 0xf1},
  };
  constexpr uint8_t zero[HAUTEUR_CHIFFRE] = {14, 27, 17, 17, 17, 27, 14};
  for (size_t i = 0; i < NB_COLONNES_CODE; i++) {
    const auto &c = COLONNES_CODE[i];
    const int tete = pluie_matrix.phase[i] / PAS_CODE;
    const int cycle = 2 * c.periode;
    for (int y = pluie_matrix.origine(i), rang = 0; y + HAUTEUR_CODE <= BAS_DEBUT; y += PAS_CODE, rang++) {
      if (masque.rangs[i] & (uint32_t(1) << rang)) continue;
      const int position = (tete + cycle - rang % cycle) % cycle;
      const int distance = position % c.periode;
      if (distance >= c.longueur) continue;
      const int niveau = distance ? std::max(0, 3 - (distance - 1) / 2) : 4;
      // Le chiffre descend avec sa traînée pendant les deux passages du cycle.
      uint32_t cle = position + 37 * i;
      cle = (cle ^ (cle >> 4)) * 0x45d9f3bu;
      dessiner_pixels(it, c.x, y, (cle >> 31) ? PIXELS[1] : zero, PIXEL_CODE, LARGEUR_CHIFFRE, teintes[niveau]);
    }
  }
}

inline uint32_t lire_utf8(const std::string &texte, size_t &i) {
  uint32_t c = static_cast<uint8_t>(texte[i++]);
  if (c < 0x80) return c;
  const int suite = c >= 0xC2 && c <= 0xDF ? 1 :
                   c >= 0xE0 && c <= 0xEF ? 2 :
                   c >= 0xF0 && c <= 0xF4 ? 3 : 0;
  if (!suite || texte.size() - i < size_t(suite)) return '?';
  c &= 0x7F >> (suite + 1);
  for (int j = 0; j < suite; j++) {
    const auto octet = static_cast<uint8_t>(texte[i]);
    if ((octet & 0xC0) != 0x80) return '?';
    c = (c << 6) | (octet & 0x3F);
    i++;
  }
  return c;
}

inline std::string normaliser_alerte(const std::string &texte) {
  std::string resultat;
  for (size_t i = 0; i < texte.size();) {
    uint32_t c = lire_utf8(texte, i);
    // Accents combinants et espaces insécables des textes copiés depuis HA.
    if (c >= 0x300 && c <= 0x36F) continue;
    if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (c >= 0xE0 && c <= 0xFE && c != 0xF7) c -= 0x20;  // Latin-1 minuscule.
    if (c == ' ' || (c >= '\t' && c <= '\r') || c == 0xA0 || c == 0x202F) {
      if (!resultat.empty() && resultat.back() != ' ') resultat += ' ';
    } else if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               (c > 0 && c < 128 && std::strchr(".:!-'?", c))) {
      resultat += char(c);
    } else if (c >= 0xC0 && c <= 0xC5) {
      resultat += 'A';
    } else if (c >= 0xC8 && c <= 0xCB) {
      resultat += 'E';
    } else if (c >= 0xCC && c <= 0xCF) {
      resultat += 'I';
    } else if ((c >= 0xD2 && c <= 0xD6) || c == 0xD8) {
      resultat += 'O';
    } else if (c >= 0xD9 && c <= 0xDC) {
      resultat += 'U';
    } else {
      switch (c) {
        case 0xC6: resultat += "AE"; break;
        case 0xC7: resultat += 'C'; break;
        case 0xD1: resultat += 'N'; break;
        case 0xDD: case 0xFF: case 0x178: resultat += 'Y'; break;
        case 0x152: case 0x153: resultat += "OE"; break;
        case 0x2018: case 0x2019: resultat += '\x27'; break;
        case 0x2010: case 0x2011: case 0x2013: case 0x2014: resultat += '-'; break;
        case 0x2026: resultat += "..."; break;
        default: resultat += '?'; break;
      }
    }
  }
  if (!resultat.empty() && resultat.back() == ' ') resultat.pop_back();
  return resultat;
}

inline constexpr int ALERTE_X = 72, ALERTE_MARGE = 12;
inline constexpr int LARGEUR_ALERTE = LARGEUR - ALERTE_X - ALERTE_MARGE;

inline void composer_alerte(Alerte &a, const std::string &texte) {
  const auto normal = normaliser_alerte(texte);
  a.lignes[1].clear();
  // Deux rangées de sept pixels et une rangée d'espacement.
  constexpr int DEUX_LIGNES = 2 * HAUTEUR_CHIFFRE + 1;
  for (int bloc = BLOC_HEURE; bloc >= 1; bloc--) {
    const size_t capacite = (LARGEUR_ALERTE + bloc) / (PAS_GLYPHE * bloc);
    if (normal.size() <= capacite) {
      a.lignes[0] = normal;
      a.bloc = bloc;
      return;
    }
    if (DEUX_LIGNES * bloc > HAUTEUR - BAS_DEBUT || normal.size() > 2 * capacite + 1) continue;
    size_t coupure = normal.rfind(' ', normal.size() / 2);
    if (coupure == std::string::npos || normal.size() - coupure - 1 > capacite) {
      coupure = normal.find(' ', normal.size() / 2);
    }
    if (coupure > capacite) continue;
    a.lignes[0] = normal.substr(0, coupure);
    a.lignes[1] = normal.substr(coupure + 1);
    a.bloc = bloc;
    return;
  }
  // Cas extrêmes : un mot trop long ou un texte qui dépasse les deux lignes.
  constexpr size_t CAPACITE = (LARGEUR_ALERTE + 1) / PAS_GLYPHE;
  const size_t espace = normal.rfind(' ', CAPACITE);
  const size_t coupure = espace == std::string::npos ? CAPACITE : espace;
  a.lignes[0] = normal.substr(0, coupure);
  a.lignes[1] = normal.substr(coupure + (coupure == espace), CAPACITE);
  if (normal.size() > coupure + (coupure == espace) + CAPACITE) {
    a.lignes[1].replace(CAPACITE - 3, 3, "...");
  }
  a.bloc = 1;
}

inline void recevoir_alerte(const std::string &texte, const std::string &icone, int secondes) {
  alerte.effacer();
  // Zéro est réservé au BAS sans alerte.
  if (++alerte.revision == 0) ++alerte.revision;
  alerte.icone = icone == "stef" ? Icone::STEF : icone == "attention" ? Icone::ATTENTION : Icone::INFO;
  composer_alerte(alerte, texte);
  // Reste dans la plage signée de l'ordonnanceur, en millisecondes.
  alerte.duree_ms = 1000u * std::clamp<int32_t>(secondes, 1, std::numeric_limits<int32_t>::max() / 1000);
}

inline void dessiner_icone_alerte(esphome::display::Display &it, bool claire) {
  // Efface et redessine les 40 × 40 px de l'icône, à 2 Hz.
  constexpr int x = 16, y = BAS_DEBUT + 8, p = 5;
  it.filled_rectangle(x, y, 8 * p, 8 * p, C.BG);
  const auto forte = alerte.icone == Icone::STEF ? C.ECOUTE :
                     alerte.icone == Icone::ATTENTION ? C.AVERTISSEMENT : C.REPONSE;
  const esphome::Color col = claire ? forte : esphome::Color(forte.r * 2 / 3, forte.g * 2 / 3, forte.b * 2 / 3);
  const auto b = [&it](int dx, int dy, int w, int h, esphome::Color couleur) {
    it.filled_rectangle(x + dx * p, y + dy * p, w * p, h * p, couleur);
  };
  if (alerte.icone == Icone::ATTENTION) {
    b(3, 0, 2, 2, col);
    b(2, 2, 4, 2, col);
    b(1, 4, 6, 2, col);
    b(0, 6, 8, 2, col);
    b(3, 2, 2, 3, C.BG);
    b(3, 6, 2, 1, C.BG);
  } else if (alerte.icone == Icone::INFO) {
    b(2, 0, 4, 8, col);
    b(1, 1, 6, 6, col);
    b(0, 2, 8, 4, col);
    b(3, 1, 2, 1, C.BG);
    b(3, 3, 2, 4, C.BG);
  } else {
    b(3, 0, 2, 5, col);
    b(3, 6, 2, 2, col);
  }
}

inline void dessiner_alerte(esphome::display::Display &it, bool claire) {
  dessiner_icone_alerte(it, claire);
  const int bloc = alerte.bloc;
  const int lignes = alerte.lignes[1].empty() ? 1 : 2;
  const int pas_y = (HAUTEUR_CHIFFRE + 1) * bloc;
  const int y = BAS_DEBUT + (HAUTEUR - BAS_DEBUT - (lignes * pas_y - bloc)) / 2;
  for (int i = 0; i < lignes; i++) {
    int x = ALERTE_X;
    for (const char c : alerte.lignes[i]) {
      dessiner_glyphe(it, x, y + i * pas_y, c, bloc, LARGEUR_CHIFFRE, C.TEXTE);
      x += PAS_GLYPHE * bloc;
    }
  }
}

inline int decalage_heure(const Haut &haut) {
  // Toutes les dix minutes, repasse par zéro pour éviter un saut de 4 px.
  constexpr int pas[] = {-DECALAGE_HEURE, 0, DECALAGE_HEURE, 0};
  return haut.minute < 0 ? 0 : pas[haut.minute / 10 % 4];
}

inline char chiffre_heure(int minute, int i) {
  if (minute < 0) return '-';
  const int valeur = i < 2 ? minute / 60 : minute % 60;
  return '0' + (i % 2 == 0 ? valeur / 10 : valeur % 10);
}

// Renvoie toute la ligne de l'heure (rectangle y = 8, hauteur 52), puis la rend pleine largeur.
inline void dessiner_ligne_heure(esphome::display::Display &it, const Haut &haut) {
  const int d = decalage_heure(haut);
  it.filled_rectangle(16, 8, 176, 52, C.BG);
  const auto col = haut.minute < 0 ? C.DISCRET : C.TEXTE;
  for (int i = 0; i < CHIFFRES_HEURE; i++) {
    const int x = HEURE_X + d + i * PAS_GLYPHE * BLOC_HEURE + (i >= 2 ? ESPACE_DEUX_POINTS : 0);
    dessiner_glyphe(it, x, HEURE_Y + d, chiffre_heure(haut.minute, i), BLOC_HEURE, LARGEUR_CHIFFRE, col);
  }
  if (haut.deux_points) {
    for (int ligne : {2, 4}) {
      it.filled_rectangle(DEUX_POINTS_X + d, HEURE_Y + d + ligne * BLOC_HEURE, BLOC_HEURE, BLOC_HEURE, col);
    }
  }
}

inline void dessiner_deux_points(esphome::display::Display &it, const Haut &haut) {
  const int d = decalage_heure(haut);
  const auto col = !haut.deux_points ? C.BG : (haut.minute < 0 ? C.DISCRET : C.TEXTE);
  for (int ligne : {2, 4}) {
    it.filled_rectangle(DEUX_POINTS_X + d, HEURE_Y + d + ligne * BLOC_HEURE, BLOC_HEURE, BLOC_HEURE, col);
  }
}

// Le pilote mipi_spi en QSPI envoie chaque ligne d'un transfert plus étroit que l'écran avec une
// nouvelle commande d'écriture : toutes les lignes retombent sur la première, l'image est brouillée.
// Seuls les transferts pleine largeur sont corrects. Deux pixels noirs aux bords gauche et droit
// (fond noir pur, jamais dessiné ici) élargissent la boîte englobante à tout l'écran.
inline void pleine_largeur(esphome::display::Display &it, int y0, int y1) {
  for (const int y : {y0, y1}) {
    it.draw_pixel_at(0, y, C.BG);
    it.draw_pixel_at(LARGEUR - 1, y, C.BG);
  }
}

inline void dessiner_heure(esphome::display::Display &it, const Haut &haut, Haut &precedent, bool) {
  dessiner_ligne_heure(it, haut);
  precedent.deux_points = haut.deux_points;
  precedent.minute = haut.minute;
}

inline void dessiner_date(esphome::display::Display &it, const Haut &haut, bool complet) {
  if (!complet) it.filled_rectangle(DATE_X, DATE_Y, DATE_LARGEUR, HAUTEUR_CHIFFRE * BLOC_DATE, C.BG);
  char date[6] = "--/--";
  if (haut.jour >= 1 && haut.jour <= 31 && haut.mois >= 1 && haut.mois <= 12) {
    std::snprintf(date, sizeof(date), "%02d/%02d", haut.jour, haut.mois);
  }
  for (int i = 0; i < 5; i++) {
    const int x = DATE_X + i * PAS_GLYPHE * BLOC_DATE;
    dessiner_glyphe(it, x, DATE_Y, date[i], BLOC_DATE, LARGEUR_CHIFFRE, C.DISCRET);
  }
}

inline void dessiner_minuteur(esphome::display::Display &it, int secondes) {
  it.filled_rectangle(MINUTEUR_X, MINUTEUR_Y, MINUTEUR_LARGEUR, HAUTEUR_CHIFFRE * MINUTEUR_BLOC, C.BG);
  if (secondes >= 0) {
    const int min = secondes / 60, sec = secondes % 60;
    const char texte[] = {char('0' + min / 10), char('0' + min % 10), ':',
                          char('0' + sec / 10), char('0' + sec % 10), '\0'};
    dessiner_texte_pixel(it, MINUTEUR_X, MINUTEUR_Y, texte, MINUTEUR_BLOC, C.DISCRET);
  }
  pleine_largeur(it, MINUTEUR_Y, MINUTEUR_Y + HAUTEUR_CHIFFRE * MINUTEUR_BLOC - 1);
}

inline constexpr int THERMOMETRE_LARGEUR = 17, THERMOMETRE_HAUTEUR = 32;

inline void dessiner_thermometre(esphome::display::Display &it, int x, int y, int dixiemes) {
  const auto b = [&it, x, y](int dx, int dy, int w, int h, esphome::Color col) {
    it.filled_rectangle(x + dx, y + dy, w, h, col);
  };
  const bool connu = dixiemes != TEMP_INCONNUE;
  const auto cadre = connu ? C.DISCRET : C.PISTE;
  b(4, 0, 6, 27, cadre);
  b(2, 21, 10, 10, cadre);
  b(0, 24, 14, 5, cadre);
  b(4, 31, 6, 1, cadre);
  b(6, 2, 2, 24, C.BG);
  for (int i = 0; i < 5; i++) {
    b(12, 2 + 4 * i, i % 2 ? 2 : 5, 1, cadre);
  }
  const auto mercure = !connu ? C.BG : dixiemes < 180 ? C.REPONSE :
                       dixiemes <= 240 ? C.ECOUTE : C.REFLEXION;
  b(3, 24, 8, 4, mercure);
  b(5, 23, 4, 7, mercure);
  if (!connu) return;
  constexpr int MIN_MERCURE = 100, MAX_MERCURE = 300;  // Échelle de 10 à 30 °C en dixièmes.
  const int niveau = (std::clamp(dixiemes, MIN_MERCURE, MAX_MERCURE) - MIN_MERCURE) * 21 /
                     (MAX_MERCURE - MIN_MERCURE);
  if (niveau) b(6, 23 - niveau, 2, niveau, mercure);
}

inline void dessiner_temperature(esphome::display::Display &it, int dixiemes) {
  constexpr int espace = 9, bloc = 4, petit = 3;
  constexpr int centre_y = (BAS_DEBUT + HAUTEUR) / 2;
  constexpr int texte_y = centre_y - HAUTEUR_CHIFFRE * bloc / 2;
  constexpr int unite_y = texte_y + HAUTEUR_CHIFFRE * (bloc - petit);
  const bool connu = dixiemes != TEMP_INCONNUE;
  char texte[16] = "--.-";
  if (connu) {
    const int valeur = std::abs(dixiemes);
    std::snprintf(texte, sizeof(texte), "%s%d.%d", dixiemes < 0 ? "-" : "", valeur / 10, valeur % 10);
  }
  const int chiffres = std::strlen(texte) - 1;  // Le point n'occupe qu'une colonne.
  const int largeur = chiffres * PAS_GLYPHE * bloc + 2 * bloc + (3 + 1 + 5) * petit;
  const int gauche = (LARGEUR - THERMOMETRE_LARGEUR - espace - largeur) / 2;
  dessiner_thermometre(it, gauche, centre_y - THERMOMETRE_HAUTEUR / 2, dixiemes);
  int x = gauche + THERMOMETRE_LARGEUR + espace;
  const auto col = connu ? C.TEXTE : C.DISCRET;
  for (const char *c = texte; *c; ++c) {
    const int colonnes = *c == '.' ? 1 : LARGEUR_CHIFFRE;
    dessiner_glyphe(it, x, texte_y, *c, bloc, colonnes, col);
    x += (colonnes + 1) * bloc;
  }
  dessiner_glyphe(it, x, unite_y, 'o', petit, 3, col);
  dessiner_glyphe(it, x + 4 * petit, unite_y, 'C', petit, 5, col);
}

inline constexpr int PASTILLES_Y = BAS_DEBUT + 4, PASTILLES_H = 10;
inline constexpr int PAS_ANIMATION_VOCALE = 250, NIVEAUX_MICRO = 8;

inline void dessiner_pastilles_vocales(esphome::display::Display &it, const Bas &bas) {
  constexpr int largeur = 120, espace = 12;
  constexpr int gauche = (LARGEUR - 3 * largeur - 2 * espace) / 2;
  constexpr Etat etapes[] = {Etat::ECOUTE, Etat::REFLEXION, Etat::REPONSE};
  constexpr esphome::Color couleurs[] = {C.ECOUTE, C.REFLEXION, C.REPONSE};
  for (int i = 0; i < 3; i++) {
    const int x = gauche + i * (largeur + espace);
    const bool active = bas.etat == etapes[i];
    const bool passee = !bas.annonce && etapes[i] < bas.etat;
    const auto col = active ? couleurs[i] : passee ? C.VALIDE : C.PISTE;
    it.filled_rectangle(x + 3, PASTILLES_Y, largeur - 6, PASTILLES_H, col);
    it.filled_rectangle(x + 1, PASTILLES_Y + 1, largeur - 2, PASTILLES_H - 2, col);
    it.filled_rectangle(x, PASTILLES_Y + 3, largeur, PASTILLES_H - 6, col);
    if (active && bas.etat == Etat::ECOUTE) {
      constexpr int piste = largeur - 12;
      const int niveau = bas.animation * piste / NIVEAUX_MICRO;
      if (niveau < piste) {
        it.filled_rectangle(x + 6 + niveau, PASTILLES_Y + 3, piste - niveau, 4, C.VALIDE);
      }
    } else if (active && bas.etat == Etat::REFLEXION) {
      it.filled_rectangle(x + 6 + 12 * bas.animation, PASTILLES_Y + 3, 12, 4, C.TEXTE);
    }
  }
}

inline void dessiner_message_vocal(esphome::display::Display &it, esphome::font::Font *petite, Etat etat) {
  using esphome::display::TextAlign;
  constexpr uint8_t micro[] = {24, 36, 36, 165, 165, 126, 24};
  constexpr uint8_t ampoule[] = {60, 66, 129, 129, 66, 60, 24};
  constexpr uint8_t haut_parleur[] = {
      0b00010010, 0b00111001, 0b11110101, 0b11110101, 0b11110101, 0b00111001, 0b00010010,
  };
  constexpr int centre_y = BAS_DEBUT + 36, bloc = 3, colonnes = 8, marge = 10;
  constexpr int icone = colonnes * bloc;
  const char *texte = etat == Etat::ECOUTE ? "J’écoute…" :
                      etat == Etat::REFLEXION ? "Je réfléchis…" : "Je réponds…";
  const auto col = etat == Etat::ECOUTE ? C.ECOUTE : etat == Etat::REFLEXION ? C.REFLEXION : C.REPONSE;
  const auto pixels = etat == Etat::ECOUTE ? micro : etat == Etat::REFLEXION ? ampoule : haut_parleur;
  const int centre_texte = (LARGEUR + icone + marge) / 2;
  int x, y, largeur, hauteur;
  it.get_text_bounds(centre_texte, centre_y, texte, petite, TextAlign::CENTER, &x, &y, &largeur, &hauteur);
  dessiner_pixels(it, x - icone - marge, centre_y - icone / 2, pixels, bloc, colonnes, col);
  it.print(centre_texte, centre_y, petite, C.TEXTE, TextAlign::CENTER, texte);
}

inline void dessiner_bas(esphome::display::Display &it, esphome::font::Font *petite, const Bas &bas) {
  it.start_clipping(0, BAS_DEBUT, LARGEUR, HAUTEUR);
  if (bas.revision_alerte) {
    dessiner_alerte(it, bas.icone_claire);
  } else if (bas.etat == Etat::CLAWD) {
    dessiner_temperature(it, bas.temperature);
  } else if (bas.etat != Etat::INCONNU) {
    dessiner_pastilles_vocales(it, bas);
    dessiner_message_vocal(it, petite, bas.etat);
  }
  it.end_clipping();
}

}  // namespace clawd_detail

inline clawd_detail::Phase clawd_phase(int heure, int soir_debut) {
  using namespace clawd_detail;
  return (heure >= soir_debut || heure < HEURE_DEBUT_JOUR) ? Phase::SOIR : Phase::JOUR;
}

inline float clawd_luminosite(clawd_detail::Phase phase, bool lampe_allumee) {
  using namespace clawd_detail;
  if (phase == Phase::JOUR) return 1.0f;
  return lampe_allumee ? SOIR_LAMPE_ALLUMEE : SOIR_LAMPE_ETEINTE;
}

inline void clawd_invalider(bool ecran_entier = true) {
  auto &a = clawd_detail::affichage;
  if (ecran_entier) {
    a.complet = true;
    a.brossage_invalide = true;
  }
  else a.milieu_invalide = true;
  clawd_detail::animation.repos.actif = false;
}

inline void clawd_fermer_menu() {
  using namespace clawd_detail;
  if (!menu.ouvert) return;
  menu.ouvert = menu.doigt_pose = false;
  menu.ferme_a = esphome::millis();
  clawd_invalider(false);
}

inline bool depart_bord(int x) { return x < clawd_detail::BORD_BALAYAGE; }

inline bool clawd_appuyer(uint32_t t, int x, int y, uint8_t doigt) {
  using namespace clawd_detail;
  auto &g = interaction;
  // Juste après la fermeture du menu, un toucher ne lance pas l'écoute.
  if (t - menu.ferme_a < 1200) {
    g.appui = false;
    return false;
  }
  // Un toucher sur la bande de l'heure (en haut) ouvre le menu.
  if (y < HAUT_FIN) {
    g.appui = false;
    menu.ouvert = true;
    menu.toucher(t);
    clawd_invalider(false);
    return false;
  }
  g.appui = true;
  g.appui_perso = y >= HAUT_FIN && y < BAS_DEBUT;
  g.calin_joue = false;
  g.debut_appui = t;
  g.doigt = doigt;
  g.x_depart = x;
  g.y_depart = y;
  g.deplacement_max_carre = 0;
  g.balayage = false;
  g.suivre(x, y);
  g.reaction = Reaction::AUCUNE;
  return g.appui_perso && !depart_bord(x);
}

inline bool clawd_suivre_appui(uint32_t t, int x, int y) {
  using namespace clawd_detail;
  auto &g = interaction;
  if (!g.appui) return false;
  g.suivre(x, y);
  // Le sens du capteur tactile est incertain : le bord gauche et le bord droit ouvrent le menu.
  const int avance = g.x_depart < BORD_BALAYAGE ? g.x_courant - g.x_depart
                     : g.x_depart >= LARGEUR - 60 ? g.x_depart - g.x_courant : 0;
  if (avance < COURSE_BALAYAGE || t - g.debut_appui >= DUREE_BALAYAGE) return false;
  g.balayage = true;
  g.appui = false;
  menu.ouvert = true;
  menu.toucher(t);
  clawd_invalider(false);
  return true;
}

inline clawd_detail::ChoixMenu clawd_toucher_menu(uint32_t t, int x, int y) {
  using namespace clawd_detail;
  menu.toucher(t);
  if (x >= DENTS_X && x < DENTS_X + DENTS_LARGEUR && y >= DENTS_Y && y < DENTS_Y + DENTS_HAUTEUR) {
    minuteur.lancer_pour(t, DUREE_BROSSAGE, Minuteur::Mode::BROSSAGE);
    interaction.reaction = Reaction::AUCUNE;
    clawd_fermer_menu();
    return ChoixMenu::RIEN;
  }
  const int dy = y - TUILE_Y;
  const int rang = dy < 0 ? -1 : dy / TUILE_PAS;
  const bool dans_rang = dy >= 0 && dy % TUILE_PAS < TUILE_HAUTEUR;
  if (rang == 1 && dans_rang) {
    for (size_t i = 0; i < std::size(AJUST_X); ++i) {
      if (x >= AJUST_X[i] && x < AJUST_X[i] + AJUST_LARGEUR) {
        minuteur.choix = std::clamp(minuteur.choix + AJUST_PAS[i], 1, 99);
        return ChoixMenu::RIEN;
      }
    }
  } else if (rang == 2 && dans_rang) {
    if (x >= TUILE_X && x < TUILE_X + (minuteur.duree ? DEMARRER_LARGEUR : TUILE_LARGEUR)) {
      minuteur.lancer(t, minuteur.choix);
      clawd_fermer_menu();
      return ChoixMenu::RIEN;
    }
    if (minuteur.duree && x >= ANNULER_X && x < ANNULER_X + ANNULER_LARGEUR) {
      minuteur.effacer();
      clawd_fermer_menu();
      return ChoixMenu::RIEN;
    }
  } else if (rang == 0 && dans_rang) {
    return ChoixMenu::RIEN;  // L'affichage de la durée ne fait rien.
  }
  clawd_fermer_menu();  // Barre RETOUR ou toucher hors des boutons.
  return ChoixMenu::RIEN;
}

inline void clawd_appui_long(uint32_t t) {
  using namespace clawd_detail;
  auto &g = interaction;
  if (!g.appui || !g.appui_perso || g.calin_joue || g.a_derive() || depart_bord(g.x_depart) ||
      t - g.debut_appui < APPUI_LONG) return;
  g.calin_joue = true;
  g.jouer(Reaction::CALIN, t);
}

inline bool clawd_relacher(uint32_t t) {
  using namespace clawd_detail;
  auto &g = interaction;
  menu.relacher(t);
  if (!g.appui) return false;
  // Traite aussi un relâchement reçu avant l'exécution du script des 700 ms.
  clawd_appui_long(t);
  g.appui = false;
  // Entre 600 et 699 ms, le relâchement termine uniquement le suivi du doigt.
  return !g.balayage && !depart_bord(g.x_depart) && !g.a_derive() && t - g.debut_appui < TOUCHER_COURT;
}

inline int clawd_temperature(float valeur) {
  using clawd_detail::TEMP_INCONNUE;
  // Rejette les valeurs non représentables avant l'arrondi en dixièmes.
  const double dixiemes = std::round(valeur * 10.0f);
  if (!std::isfinite(dixiemes) || dixiemes <= TEMP_INCONNUE ||
      dixiemes > std::numeric_limits<int>::max()) return TEMP_INCONNUE;
  return static_cast<int>(dixiemes);
}

inline uint16_t clawd_actualiser_bas(clawd_detail::Etat etat, bool annonce, uint32_t t, float temperature, float micro) {
  using namespace clawd_detail;
  auto &a = affichage;
  a.bas = {};
  if (etat == Etat::CLAWD && alerte.active) {
    a.bas.revision_alerte = alerte.revision;
    // Deux cycles clair/atténué par seconde, sur la seule surface de l'icône.
    a.bas.icone_claire = ((t - alerte.debut) / 250) % 2 == 0;
  } else {
    a.bas.etat = etat;
    a.bas.annonce = annonce;
    if (etat == Etat::CLAWD) {
      a.bas.temperature = clawd_temperature(temperature);
    } else if (etat == Etat::ECOUTE && std::isfinite(micro)) {
      a.bas.animation = std::lround(std::clamp(micro, 0.0f, 1.0f) * NIVEAUX_MICRO);
    } else if (etat == Etat::REFLEXION) {
      const int pas = (t - a.debut_etat) / PAS_ANIMATION_VOCALE % 16;
      a.bas.animation = std::min(pas, 16 - pas);
    }
  }
  if (a.bas.contenu_change(a.bas_dessine) &&
      (a.bas.etat != a.bas_dessine.etat || a.bas_disponible(t))) return BAS;
  if (a.bas.animation != a.bas_dessine.animation && t - a.dernier_bas >= PAS_ANIMATION_VOCALE) {
    return PASTILLES_VOCALES;
  }
  if (a.bas.icone_claire != a.bas_dessine.icone_claire && a.bas_disponible(t)) return ICONE_ALERTE;
  return 0;
}

inline uint16_t clawd_tick(clawd_detail::Etat suivant, uint32_t t, bool vocal, bool interface_libre, bool annonce,
                           int minute, int seconde, int jour, int mois, bool reactions, float temperature,
                           float micro, clawd_detail::Costume costume) {
  using namespace clawd_detail;
  auto &a = affichage;
  auto &repos = animation.repos;
  if (!interface_libre || menu.inactif(t)) clawd_fermer_menu();
  const bool entree_ecoute = !vocal && suivant == Etat::ECOUTE && a.etat != suivant;
  if (a.etat != suivant) {
    a.etat = suivant;
    a.debut_etat = t;
  }
  a.t = t;
  const bool brossage = minuteur.mode == Minuteur::Mode::BROSSAGE;
  if (a.ecran_brossage && !brossage) {
    a.ecran_brossage = false;
    clawd_invalider();
  }
  if (reactions) repos.climat = mesurer_climat(repos.climat, temperature);
  else repos.couper_reactions();
  if (vocal) {
    a.brossage_invalide = true;
    repos.actif = false;
    pluie_matrix.en_pause = true;
    // Pendant l'audio, seuls les transferts de la bande vocale sont autorisés.
    if (suivant == Etat::CLAWD && !annonce) return a.zones = 0;
    return a.zones = clawd_actualiser_bas(annonce ? Etat::REPONSE : suivant, annonce, t, temperature, micro);
  }

  if (brossage) {
    repos.actif = false;
    pluie_matrix.en_pause = true;
    if (!interface_libre) return a.zones = 0;
    a.brossage = minuteur.ecoule ? DUREE_BROSSAGE : t - minuteur.debut;
    const auto precedent = a.brossage_dessine;
    if (!a.ecran_brossage || a.brossage_invalide ||
        (a.brossage == DUREE_BROSSAGE && precedent != a.brossage)) return a.zones = BROSSAGE_COMPLET;
    if (a.brossage == DUREE_BROSSAGE) return a.zones = 0;
    a.zones = 0;
    if (pas_brosse(a.brossage) != pas_brosse(precedent)) a.zones |= BROSSAGE_PERSO;
    if (a.brossage / 1000 != precedent / 1000) a.zones |= BROSSAGE_TEMPS;
    if (a.brossage / DUREE_ZONE_DENTS != precedent / DUREE_ZONE_DENTS ||
        changement_dents(a.brossage) != changement_dents(precedent)) a.zones |= BROSSAGE_CONSIGNE;
    return a.zones;
  }

  // Attend la fin du son avant un redessin complet.
  const bool libre = suivant == Etat::CLAWD;
  const bool complet = interface_libre && a.complet && !menu.ouvert;
  const bool matrix = costume == Costume::MATRIX;
  if (!libre || !matrix || menu.ouvert || (a.milieu_invalide && !interface_libre)) pluie_matrix.en_pause = true;

  // L'heure reste fixe pendant l'échange. Sinon seul le « : » clignote entre deux minutes.
  a.haut = interface_libre ? Haut{minute, jour, mois, minute < 0 || seconde % 2 == 0} : a.haut_dessine;
  uint16_t zones = complet ? COMPLET : 0;
  if (!complet) {
    if (a.haut.minute != a.haut_dessine.minute) zones |= HEURE;
    else if (a.haut.deux_points != a.haut_dessine.deux_points) zones |= DEUX_POINTS;
    if (a.haut.jour != a.haut_dessine.jour || a.haut.mois != a.haut_dessine.mois) zones |= DATE;
  }

  if (menu.ouvert && interface_libre) {
    a.tuiles = {false, minuteur.duree != 0, Costume::AUCUN, minuteur.choix};
    if (a.milieu_invalide || a.tuiles.choix != a.tuiles_dessinees.choix) zones |= MENU_MINUTEUR;
    if (a.milieu_invalide) zones |= MENU_BUREAU;
    if (a.milieu_invalide || a.tuiles.minuteur != a.tuiles_dessinees.minuteur) zones |= MENU_DEGUISEMENT;
  } else if (!menu.ouvert && (!a.milieu_invalide || interface_libre) && (libre || entree_ecoute)) {
    calcul_pose(suivant, t, reactions, costume);
    if (complet || a.milieu_invalide || t - a.dernier_perso >= PAS_PERSO) {
      const bool pluie_change = libre && matrix && pluie_matrix.avancer(t);
      if (a.milieu_invalide || pluie_change ||
          std::memcmp(&animation.courante, &animation.dessinee, sizeof(Frame)) != 0) zones |= PERSO;
    }
  }
  if (interface_libre && !menu.ouvert) {
    a.minuteur = minuteur.affichage(t);
    if (a.minuteur != a.minuteur_dessine) zones |= MINUTEUR;
  }

  const uint16_t zones_bas = clawd_actualiser_bas(suivant, false, t, temperature, micro);
  if (!complet) zones |= zones_bas;
  return a.zones = zones;
}

inline void clawd_dessiner(esphome::display::Display &it, esphome::font::Font *petite) {
  using namespace clawd_detail;
  auto &a = affichage;
  uint16_t zones = a.zones;
  if (zones & (BROSSAGE_COMPLET | BROSSAGE_TEMPS | BROSSAGE_PERSO | BROSSAGE_CONSIGNE)) {
    constexpr int fin_temps = BROSSAGE_TEMPS_Y + HAUTEUR_CHIFFRE * BROSSAGE_TEMPS_BLOC;
    if (zones & BROSSAGE_COMPLET) {
      it.filled_rectangle(0, 0, LARGEUR, HAUTEUR, C.BG);
      if (a.brossage == DUREE_BROSSAGE) {
        dessiner_bravo(it);
      } else {
        dessiner_temps_brossage(it, a.brossage);
        dessiner_perso_brossage(it, a.brossage);
        dessiner_consigne_brossage(it, a.brossage);
      }
      a.ecran_brossage = true;
      a.brossage_invalide = false;
      a.zones = 0;
    } else if (zones & BROSSAGE_TEMPS) {
      it.filled_rectangle(0, BROSSAGE_TEMPS_Y, LARGEUR, fin_temps - BROSSAGE_TEMPS_Y, C.BG);
      dessiner_temps_brossage(it, a.brossage);
      a.zones &= ~BROSSAGE_TEMPS;
    } else if (zones & BROSSAGE_PERSO) {
      // La bande couvre toute la course des bulles et de la tête inclinée.
      constexpr int debut = 6 * T - 12, fin = 15 * T + 12;
      it.filled_rectangle(0, debut, LARGEUR, fin - debut, C.BG);
      it.start_clipping(0, debut, LARGEUR, fin);
      dessiner_perso_brossage(it, a.brossage);
      it.end_clipping();
      a.zones &= ~BROSSAGE_PERSO;
    } else {
      it.filled_rectangle(0, BROSSAGE_CONSIGNE_Y, LARGEUR, HAUTEUR - BROSSAGE_CONSIGNE_Y, C.BG);
      dessiner_consigne_brossage(it, a.brossage);
      a.zones &= ~BROSSAGE_CONSIGNE;
    }
    if (!a.zones) a.brossage_dessine = a.brossage;
    return;
  }
  const bool complet = zones & COMPLET;
  // Hors redessin complet, isoler les petits transferts du haut des animations.
  if (complet) {
    zones = HEURE | DATE | PERSO | BAS | MINUTEUR;
  } else {
    if (zones & MENU_MINUTEUR) zones = MENU_MINUTEUR;
    else if (zones & MENU_BUREAU) zones = MENU_BUREAU;
    else if (zones & MENU_DEGUISEMENT) zones = MENU_DEGUISEMENT;
    else if ((zones & PERSO) && a.milieu_invalide) zones = PERSO;
    else if (zones & MINUTEUR) zones = MINUTEUR;
    else if (zones & HEURE) zones = HEURE;
    else if (zones & DATE) zones = DATE;
    else if (zones & DEUX_POINTS) zones = DEUX_POINTS;
    // Garder deux transferts distincts si BAS et PERSO changent ensemble au repos.
    else if (zones & PERSO) zones = PERSO;
  }
  if (zones == MENU_MINUTEUR || zones == MENU_BUREAU || zones == MENU_DEGUISEMENT) {
    dessiner_tuile(it, static_cast<Zone>(zones), a.tuiles);
    if (zones == MENU_MINUTEUR) a.tuiles_dessinees.choix = a.tuiles.choix;
    if (zones == MENU_DEGUISEMENT) {
      a.tuiles_dessinees.minuteur = a.tuiles.minuteur;
      a.milieu_invalide = false;
    }
  }
  if (complet) it.filled_rectangle(0, 0, LARGEUR, HAUT_FIN, C.BG);
  if (zones & PERSO) {
    it.filled_rectangle(0, HAUT_FIN, LARGEUR, BAS_DEBUT - HAUT_FIN, C.BG);
    it.start_clipping(0, HAUT_FIN, LARGEUR, BAS_DEBUT);
    dessiner_perso(it, animation.courante);
    it.end_clipping();
    animation.dessinee = animation.courante;
    a.dernier_perso = a.t;
    a.milieu_invalide = false;
  }
  if (zones & MINUTEUR) {
    if (!complet || a.minuteur >= 0) dessiner_minuteur(it, a.minuteur);
    a.minuteur_dessine = a.minuteur;
  }
  if (zones & HEURE) {
    dessiner_heure(it, a.haut, a.haut_dessine, complet);
    if (!complet) pleine_largeur(it, 8, 59);
  }
  if (zones & DATE) {
    dessiner_date(it, a.haut, complet);
    if (!complet) pleine_largeur(it, DATE_HAUT, DATE_BAS - 1);
    a.haut_dessine.jour = a.haut.jour;
    a.haut_dessine.mois = a.haut.mois;
  }
  if (zones & DEUX_POINTS) {
    dessiner_deux_points(it, a.haut);
    pleine_largeur(it, 20, 46);
    a.haut_dessine.deux_points = a.haut.deux_points;
  }
  if (zones & BAS) {
    it.filled_rectangle(0, BAS_DEBUT, LARGEUR, HAUTEUR - BAS_DEBUT, C.BG);
    dessiner_bas(it, petite, a.bas);
    a.bas_dessine = a.bas;
    a.dernier_bas = a.t;
  } else if (zones & PASTILLES_VOCALES) {
    dessiner_pastilles_vocales(it, a.bas);
    pleine_largeur(it, PASTILLES_Y, PASTILLES_Y + PASTILLES_H - 1);
    a.bas_dessine.animation = a.bas.animation;
    a.dernier_bas = a.t;
  } else if (zones & ICONE_ALERTE) {
    dessiner_icone_alerte(it, a.bas.icone_claire);
    pleine_largeur(it, BAS_DEBUT + 8, BAS_DEBUT + 47);
    a.bas_dessine.icone_claire = a.bas.icone_claire;
    a.dernier_bas = a.t;
  }
  if (complet) {
    a.complet = false;
    a.zones = 0;
  } else {
    a.zones &= ~zones;
  }
}
