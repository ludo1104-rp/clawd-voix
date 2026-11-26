# Clawd Voix

Assistant vocal local pour la carte **Waveshare ESP32-S3-Touch-AMOLED-2.16**, avec Home Assistant.
Clawd, un petit personnage animé, s'affiche à l'écran. La carte écoute, Home Assistant comprend et agit, puis la carte répond à voix haute.

Deux configurations possibles dans Home Assistant : tout en local (Whisper, commandes locales, Piper) ou via OpenAI (intégration « ChatGPT », clé API personnelle). Langue des services OpenAI : `fr-FR`.

## Fonctions
- Mot d'éveil « Okay Nabu » ou toucher de l'écran.
- Clawd animé selon l'état : repos, écoute, réflexion, réponse.
- Bouton BOOT : éteint ou allume l'écran. Bouton Key3 : lance ou annule un échange vocal.
- Charge de la batterie limitée à 4,0 V (puce AXP2101) pour limiter la chauffe.

## Installer
1. Installer [ESPHome](https://esphome.io) (2026.11).
2. Copier `secrets.exemple.yaml` en `secrets.yaml` et le remplir.
3. Téléverser : `esphome run clawd-voix.yaml`
4. Dans Home Assistant, créer un assistant (Whisper et Piper, ou OpenAI) et le choisir pour la carte.

- Minuteur : toucher l'heure pour ouvrir le menu (1 à 99 min), carillon à la fin.
- Fiche d'état en bas de l'écran et mode nuit.

## Pièges connus
- Broches des données I2S inversées dans la documentation Waveshare : micro = GPIO10, haut-parleur = GPIO8.
- Arrêter le mot d'éveil pendant l'échange (il partage le micro et le bus I2S).
- Suspendre l'animation pendant l'échange, sinon le son est haché.

[CC BY-NC-SA 4.0](LICENSE) · © 2026 Ludo (@rprof_1) · https://rprof1.com
