#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>

#include "esphome/components/display/display.h"
#include "esphome/components/font/font.h"
#include "esphome/core/helpers.h"

namespace clawd_detail {

inline constexpr int T = 24;
inline constexpr uint32_t SOMMEIL = 25000;

inline constexpr struct {
  esphome::Color BG, CORPS, CLAIR, OMBRE, JOUE, OEIL;
  esphome::Color LANGUE, SOL, LUN, VERRE, BLANC, REFLET;
  esphome::Color ONDE, AMP, AMPOFF, BASE, ZC;
} C{
    {0xfd, 0xf3, 0xe7}, {0xe7, 0x83, 0x4b}, {0xf2, 0x96, 0x5d}, {0xc9, 0x68, 0x3c},
    {0xd6, 0x69, 0x49}, {0x51, 0x2c, 0x26}, {0xe4, 0x83, 0x6c}, {0xea, 0xd9, 0xc7},
    {0x24, 0x1e, 0x22}, {0x39, 0x30, 0x35}, {0xff, 0xff, 0xff}, {0xb8, 0xa9, 0xad},
    {0xcd, 0xbd, 0xad}, {0xef, 0xc3, 0x53}, {0xdd, 0xca, 0xaa}, {0x9b, 0x81, 0x77},
    {0x84, 0x92, 0xb3}
};

enum class Etat { INCONNU, CLAWD, ECOUTE, REFLEXION, REPONSE, SURPRISE };
enum Mode { IDLE, BLINK, LOOK, JUMPM, WIGGLE, YAWN, DANCE, SLEEP };

inline constexpr uint32_t DURATIONS[] = {0, 360, 1100, 990, 720, 1650, 1600, 0};
inline constexpr int8_t PAS[] = {0, 1, 0, -1};
inline constexpr int8_t ZN[] = {1, 2, 2, 0};
// dy, écrasement, bras gauche, bras droit.
inline constexpr int8_t JUMP[][4] = {
    {0, 1, 1, 1}, {-1, 0, 0, 0}, {-2, 0, -1, -1},
    {-3, 0, -2, -1}, {-3, 0, -2, -1}, {-2, 0, -1, 0},
    {-1, 0, 0, 0}, {0, 1, 1, 1}, {0, 0, 0, 0}
};
// Fin cumulée de chaque pas de parole, en millisecondes.
inline constexpr std::array<uint32_t, 6> FIN_PAS_PAROLE{140, 260, 420, 550, 700, 840};

struct Frame {
  // lun relève les lunettes. yeux : 0 ouverts, 1 fermés, 2 écarquillés.
  // bouche : 0 sourire, 1 ouverte, 2 détendue.
  int8_t dx, dy, ecrase, gauche, droit, lun, yeux, regard, bouche;
  int8_t zn, montee_z, glint, ph, ampoule, ondes;
};
static_assert(std::has_unique_object_representations_v<Frame>);

struct Animation {
  Frame courante{};
  Frame dessinee{};
  bool dessin_effectue = false;
  Etat etat = Etat::INCONNU;
  Mode mode = IDLE;
  Mode prochaine_action = BLINK;
  uint32_t debut_etat = 0;
  uint32_t debut_mode = 0;
  uint32_t attente = 2500;
  int dir = 1;
};

inline Animation animation;

inline Etat lire_etat(const std::string &etat) {
  if (etat == "Clawd") return Etat::CLAWD;
  if (etat == "J'écoute...") return Etat::ECOUTE;
  if (etat == "Je réfléchis...") return Etat::REFLEXION;
  if (etat == "Je réponds...") return Etat::REPONSE;
  return Etat::SURPRISE;
}

inline void calcul_repos(Animation &a, Frame &f, uint32_t t) {
  const uint32_t ecoule = t - a.debut_etat;
  if (a.mode != SLEEP && ecoule >= SOMMEIL) {
    a.mode = SLEEP;
    a.debut_mode = a.debut_etat + SOMMEIL;
  } else if (a.mode == IDLE) {
    if (t - a.debut_mode >= a.attente) {
      a.mode = a.prochaine_action;
      a.prochaine_action = a.mode == DANCE ? BLINK : static_cast<Mode>(a.mode + 1);
      a.debut_mode = t;
      a.dir = esphome::random_uint32() % 2 ? 1 : -1;
    }
  } else if (a.mode != SLEEP && t - a.debut_mode >= DURATIONS[a.mode]) {
    a.mode = IDLE;
    a.debut_mode = t;
    a.attente = 1500 + esphome::random_uint32() % 1301;
  }

  const uint32_t e = t - a.debut_mode;
  f.glint = ecoule / 540 % 3;
  switch (a.mode) {
    case IDLE:
      break;
    case BLINK:
      f.lun = 1;
      f.yeux = e >= 100 && e < 230;
      break;
    case LOOK:
      f.lun = 1;
      if (e >= 170 && e < 910) {
        f.regard = e < 530 ? a.dir : -a.dir;
        f.dx = f.regard;
      }
      break;
    case JUMPM: {
      const auto &pas = JUMP[e / 110];
      f.dy = pas[0];
      f.ecrase = pas[1];
      f.gauche = pas[2];
      f.droit = pas[3];
      break;
    }
    case WIGGLE:
      f.dx = PAS[e / 90 % 4] * a.dir;
      f.gauche = f.dx;
      f.droit = -f.dx;
      break;
    case YAWN:
      f.lun = 1;
      f.yeux = 1;
      if (e >= 260 && e < 1320) {
        f.bouche = 1;
        f.gauche = -1;
      }
      break;
    case DANCE: {
      const int s = e / 200 % 4;
      const int cote = s < 2 ? a.dir : -a.dir;
      f.dx = s % 2 ? 0 : cote;
      f.dy = s % 2 ? -1 : 0;
      f.gauche = cote;
      f.droit = -cote;
      break;
    }
    case SLEEP: {
      const uint32_t phase_z = e / 650 % 4;
      f.lun = 1;
      f.yeux = 1;
      f.bouche = 2;
      f.glint = 0;
      f.ecrase = e / 1100 % 2;
      f.droit = f.ecrase;
      f.zn = ZN[phase_z];
      f.montee_z = f.zn ? phase_z : 0;
      break;
    }
  }
}

}  // namespace clawd_detail

inline bool clawd_tick(const std::string &etat, uint32_t t) {
  using namespace clawd_detail;
  Animation &a = animation;
  const Etat suivant = lire_etat(etat);
  if (suivant != a.etat) {
    a.etat = suivant;
    a.debut_etat = t;
    a.debut_mode = t;
    a.mode = IDLE;
    a.prochaine_action = BLINK;
    a.attente = 2500;
    a.dir = 1;
  }

  const uint32_t e = t - a.debut_etat;
  Frame f{};
  switch (a.etat) {
    case Etat::CLAWD:
      calcul_repos(a, f, t);
      break;
    case Etat::ECOUTE:
      f.lun = 1;
      f.dx = e / 900 % 2 ? 1 : -1;
      f.gauche = -1;
      f.glint = e / 270 % 3;
      f.ondes = 1 + e / 450 % 2;
      break;
    case Etat::REFLEXION:
      f.lun = 1;
      f.regard = e / 900 % 2 ? 1 : -1;
      f.droit = -1;
      f.ampoule = 1;
      f.ph = e / 430 % 2;
      f.glint = e / 540 % 3;
      break;
    case Etat::REPONSE: {
      const uint32_t phase = e % FIN_PAS_PAROLE.back();
      const auto pas = std::upper_bound(FIN_PAS_PAROLE.begin(), FIN_PAS_PAROLE.end(), phase);
      f.bouche = (pas - FIN_PAS_PAROLE.begin()) % 2 == 0;
      f.lun = 1;
      f.dy = e / 420 % 2 ? -1 : 0;
      f.glint = e / 540 % 3;
      break;
    }
    case Etat::SURPRISE:
      f.lun = 2;
      f.yeux = 2;
      f.bouche = 1;
      break;
    case Etat::INCONNU:
      break;
  }

  a.courante = f;
  return !a.dessin_effectue || std::memcmp(&a.courante, &a.dessinee, sizeof(Frame)) != 0;
}

inline void clawd_dessiner(esphome::display::Display &it, esphome::font::Font *police,
                          const std::string &etat) {
  using namespace clawd_detail;
  const Frame &f = animation.courante;
  it.fill(C.BG);
  const int X = f.dx, Y = f.dy;
  const auto b = [&it, X, Y](float x, float y, float w, float h, esphome::Color couleur) {
    it.filled_rectangle(int(std::round((x + X) * T)), int(std::round((y + Y) * T)),
                        int(std::round(w * T)), int(std::round(h * T)), couleur);
  };

  // L'ombre reste au sol et se resserre avec la hauteur du saut.
  const int retrait = -Y;
  b(4 + retrait, 17 - Y, 11 - 2 * retrait, 0.5, C.SOL);
  b(3.5 + retrait, 17 - Y, 12 - 2 * retrait, 0.25, C.SOL);

  const int front = 6 + f.ecrase;
  const int gauche = 12 + f.gauche, droit = 12 + f.droit;
  b(2, std::min(12, gauche), 2, std::abs(f.gauche) + 1, C.CORPS);
  b(1, gauche, 2, 1, C.CORPS);
  b(1, gauche + 0.75, 1, 0.25, C.OMBRE);
  b(15, std::min(12, droit), 2, std::abs(f.droit) + 1, C.CORPS);
  b(16, droit, 2, 1, C.CORPS);
  b(17, droit + 0.75, 1, 0.25, C.OMBRE);
  for (int x = 4; x <= 13; x += 3) {
    b(x, 15, 2, 2, C.OMBRE);
    b(x, 15, 2, 1.5, C.CORPS);
  }
  b(3, front, 13, 15 - front, C.CORPS);
  b(3.5, front, 12, 0.25, C.CLAIR);
  b(3, 14.5, 13, 0.5, C.OMBRE);

  // Visage et lunettes sont ancrés aux joues, y compris quand le front se tasse.
  if (f.lun) {
    for (int x = 6; x <= 12; x += 6) {
      if (f.yeux == 1) {
        b(x - 0.25, 12.25, 1.5, 0.5, C.OEIL);
      } else if (f.yeux == 2) {
        b(x - 0.25, 11.25, 1.5, 1.5, C.OEIL);
        b(x, 11.5, 0.5, 0.5, C.BLANC);
      } else {
        b(x + 0.125 + 0.5 * f.regard, 12.25, 0.75, 0.75, C.OEIL);
      }
    }
  }
  const int barre = 10 - f.lun;
  b(4, barre, 11, 1, C.LUN);
  b(4, barre, 11, 0.25, C.VERRE);
  for (int x = 4; x <= 10; x += 6) {
    b(x, barre + 1, 5, 1, C.LUN);
    b(x + 1, barre + 2, 3, 1, C.LUN);
    b(x + 0.5, barre + 1, 0.5, 0.5, f.glint == 1 ? C.REFLET : C.BLANC);
    b(x + 1, barre + 1.5, 0.5, 0.5, C.BLANC);
    b(x + 1.5, barre + 2, 0.5, 0.5, f.glint == 2 ? C.BLANC : C.REFLET);
  }
  b(5, 13.25, 1.5, 0.5, C.JOUE);
  b(12.5, 13.25, 1.5, 0.5, C.JOUE);
  if (f.bouche == 1) {
    b(8.5, 13, 2, 1.5, C.OEIL);
    b(9, 14, 1, 0.5, C.LANGUE);
  } else if (f.bouche == 2) {
    b(9, 13.75, 1, 0.5, C.OEIL);
  } else {
    b(8.25, 13.25, 0.5, 0.5, C.OEIL);
    b(10.25, 13.25, 0.5, 0.5, C.OEIL);
    b(8.75, 13.75, 1.5, 0.5, C.OEIL);
  }

  if (f.ampoule) {
    const esphome::Color col = f.ph ? C.AMPOFF : C.AMP;
    b(8.5, front - 2.5, 2, 0.5, col);
    b(8, front - 2, 3, 1.25, col);
    b(8.5, front - 0.75, 2, 0.75, C.BASE);
    b(9, front - 0.5, 1, 0.25, C.AMPOFF);
    if (!f.ph) {
      b(8.5, front - 2, 0.5, 0.5, C.BLANC);
      b(6.5, front - 1.75, 0.75, 0.25, C.AMP);
      b(11.75, front - 1.75, 0.75, 0.25, C.AMP);
    }
  }

  if (f.ondes) {
    const float retrait_onde = f.ondes == 1 ? 0 : 0.5;
    b(1.75 - retrait_onde, 7.5, 0.5, 0.25, C.ONDE);
    b(1.5 - retrait_onde, 8.25, 0.5, 0.25, C.ONDE);
    b(16.75 + retrait_onde, 7.5, 0.5, 0.25, C.ONDE);
    b(17 + retrait_onde, 8.25, 0.5, 0.25, C.ONDE);
  }

  for (int i = 0; i < f.zn; i++) {
    const float x = 12.5 + 2.5 * i, y = 3 - 2 * i - 0.25 * f.montee_z;
    b(x, y, 2, 0.5, C.ZC);
    b(x + 1, y + 0.5, 0.5, 0.5, C.ZC);
    b(x + 0.5, y + 1, 0.5, 0.5, C.ZC);
    b(x, y + 1.5, 2, 0.5, C.ZC);
  }

  if (etat != "Clawd")  // au repos, pas de texte sous le personnage
  it.printf(240, 452, police, C.OMBRE, esphome::display::TextAlign::CENTER, "%s", etat.c_str());
  animation.dessinee = f;
  animation.dessin_effectue = true;
}
